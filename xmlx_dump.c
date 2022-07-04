#include <zlib.h>
#include <libxml2/libxml/parser.h>
#include <libxml2/libxml/tree.h>

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#define NVRAM_MAGIC 0x00123123
#define NVRAM_HEADER_SIZE 40

struct nvram_header_t {
    uint32_t magic;
    uint32_t len;
    uint32_t crc;
    uint32_t orig_len;
    uint32_t version;
};

static unsigned long fsize(int fd) {
    struct stat st;
    st.st_size = 0;
    fstat(fd, &st);
    return st.st_size;
}


static void visit_xml_tree_dfs(xmlNodePtr node, char * stack[], int stack_top) {
    for(xmlNodePtr cur = node ; cur ; cur = cur->next) {
        stack[stack_top] = xmlGetProp(cur, "name");
        if(strcmp("PARAMETER", cur->name)) {
            visit_xml_tree_dfs(cur->children, stack, stack_top + 1);
        } else {
            for(int i = 1 ; i <= stack_top ; i ++) {
                printf("%s", stack[i]);
            }
            printf(" = %s\n", (char*)xmlGetProp(cur, "value") ?: "");
        }
    }
}

static void list_all_parameters_from_buffer(char * config_data, int size) {
    int ret;
    xmlDocPtr doc = NULL;
    xmlNodePtr cur = NULL;

    doc = xmlParseMemory(config_data, size);
    if(!doc) {
        ret = -1;
        printf("failed to parse from memory\n");
        goto out;
    }
    
    cur = xmlDocGetRootElement(doc);
    if(!cur) {
        printf("Empty Config\n");
        goto out;
    }
    
    char * stack[512];
    visit_xml_tree_dfs(cur, stack, 0);

out:
    if(doc) {
        xmlFreeDoc(doc);
    }
}

static void usage(const char * name) {
    printf("Usage: %s -f <compressed config file> [-l | -x]\n", name);
}

int main(int argc, char * argv[]) {
    int ret;
    int opcode;
    int dump_list = 0, dump_xml = 0;
    const char * config_file;
    
    while(-1 != (opcode = getopt(argc, argv, "f:lx"))) {
        switch(opcode) {
            case 'f':
                config_file = optarg;
                break;

            case 'l':
                dump_list = 1;
                break;

            case 'x':
                dump_xml = 1;
                break;

            case '?':
            default:
                usage(argv[0]);
                return -EINVAL;
        }
    }
    
    if(!dump_xml && !dump_list) {
        dump_list = 1;
    }

    if(argc < 2) {
        usage(argv[0]);
        return -EINVAL;
    } else if(access(config_file, R_OK)) {
        printf("file %s not found\n", config_file);
        return -EINVAL;
    }

    int fd = open(config_file, O_RDONLY);
    if(fd < 0) {
        printf("failed to open %s: %m\n", config_file);
        return -errno;
    }
    
    off_t size;
    void * data = NULL;
    void * compressed_data;
    void * decompressed_data;
    unsigned long decompressed_data_len;
    struct nvram_header_t * header;
    uint32_t crc32_cksum;
    
    
    size = fsize(fd);
    if(size < sizeof(struct nvram_header_t)) {
        ret = -EINVAL;
        printf("failed to get size of %s: %m\n", config_file);
        goto out;
    }

    data = malloc(size);
    if(!data) {
        ret = -ENOMEM;
        printf("failed to allocate memory: %m\n");
        goto out;
    }
    
    if(size != read(fd, data, size)) {
        ret = -errno;
        printf("failed to read %s: %m\n", config_file);
        goto out;
    }

    header = (struct nvram_header_t*)data;
    if(ntohl(header->magic) != NVRAM_MAGIC) {
        ret = -EINVAL;
        printf("invalid magic\n");
        goto out;
    }

    size = ntohl(header->len);
    compressed_data = data + NVRAM_HEADER_SIZE;
    crc32_cksum = crc32(0, compressed_data, size);
    if(crc32_cksum != ntohl(header->crc)) {
        ret = -EINVAL;
        printf("CRC dismatch: 0x%x != 0x%x\n", crc32_cksum, ntohl(header->crc));
        goto out;
    }
   
    decompressed_data_len = ntohl(header->orig_len);
    decompressed_data = malloc(decompressed_data_len + 1);
    ((char*)decompressed_data)[decompressed_data_len] = 0;
    if(Z_OK != uncompress(decompressed_data, &decompressed_data_len, compressed_data, size)) {
        ret = -EINVAL;
        printf("failed to uncompress data from %s\n", config_file);
        goto out;
    }
    
    if(dump_xml) {
        printf("%s\n", (char*)decompressed_data);
    }

    if(dump_list) {
        if(dump_xml) {
            printf("\n\n");
        }
        list_all_parameters_from_buffer(decompressed_data, decompressed_data_len);
    }

out:
    if(data) {
        free(data);
    }
    if(decompressed_data) {
        free(decompressed_data);
    }
    close(fd);

    return ret;
}
