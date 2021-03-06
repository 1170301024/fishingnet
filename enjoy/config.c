/*
 *      
 * Copyright (c) 2016-2019 Cisco Systems, Inc.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 *   Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * 
 *   Redistributions in binary form must reproduce the above
 *   copyright notice, this list of conditions and the following
 *   disclaimer in the documentation and/or other materials provided
 *   with the distribution.
 * 
 *   Neither the name of the Cisco Systems, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/**
 * \file config.c
 *
 * \brief implementation for the configuration system
 *
 */
#ifdef HAVE_CONFIG_H
#include "joy_config.h"
#endif
#include <stdio.h>
#include <stdlib.h>    
#include <limits.h>
#include <ctype.h> 
#include <libxml/parser.h>
#include <libxml/tree.h>

#include    "../include/config.h"
#include    "../include/error.h"

#ifdef WIN32
#include "unistd.h"
#include <ShlObj.h>

size_t getline(char **lineptr, size_t *n, FILE *stream);
#endif

/** returns if two string are the same */
#define match(c, x) (!strncmp(c, x, strlen(x)))

// only parse the value, do not care about whether valid argument is or not

// parse integer 
#define parse_intval(x, val_str, min, max) parse_int(x, val_str, 2, min, max)

// parser short
static int parse_shortval(unsigned short *x, const char *val_str,unsigned short min, unsigned short max) {
    const char *c = val_str;
    int rv;
    if (val_str == NULL) {
            return failure;
    }
    while (*c != 0) {
        if (!isdigit(*c)) {
            printf("error: argument %s must be a number ", val_str);
            return failure;
        }
        c++;
    }
    rv = atoi(val_str);
    if (rv < min || rv > max) {
        printf("error: value must be between %d and %d ", min, max);
        return failure;
    }
    *x = (unsigned short) rv;
    return ok;
}
// parse uint_8
static int parse_octval(uint8_t *x, const char *val_str, uint8_t min, uint8_t max) {
    const char *c = val_str;
    int rv;
    if (val_str == NULL) {
            return failure;
    }
    while (*c != 0) {
        if (!isdigit(*c)) {
            printf("error: argument %s must be a number ", val_str);
            return failure;
        }
        c++;
    }
    rv = atoi(val_str);
    if (rv < min || rv > max) {
        printf("error: value must be between %d and %d ", min, max);
        return failure;
    }
    *x = (uint8_t) rv;
    return ok;
}
            
// parse bool values
#define parse_boolval(x, val_str) parse_bool(x, val_str, 1)

/* parses an integer value */
static int parse_int (unsigned int *x, const char *arg, int num_arg, unsigned int min, unsigned int max) {
    const char *c = arg;

    if (x == NULL) {
        return failure;
    }

    if (num_arg == 2) {
        if (arg == NULL) {
            return failure;
        }
        while (*c != 0) {
            if (!isdigit(*c)) {
                printf("error: argument %s must be a number ", arg);
                return failure;
            }
            c++;
        }
        *x = atoi(arg);
        if (*x < min || *x > max) {
            printf("error: value must be between %d and %d ", min, max);
            return failure;
        }
    } else {
        return failure;
    }
    return ok;
}

/* parses a boolean value */
static int parse_bool (bool *x, const char *arg, int num_arg) {
    bool val = 0;
    /* if the number of arguments is one, default turn the option on */
    if (num_arg == 1) {
        *x = 1;
        return ok;
    }

    /* sanity check the length of the value string */
    if (strlen(arg) > 1) {
        printf("error: value too big, value must be 0 or 1");
        return failure;
    }

    /* make sure value is a digit */
    if (!isdigit(*arg)) {
        printf("error: non-digit, value must be 0 or 1");
        return failure;
    }

    /* change the value into a digit */
    val = atoi(arg);
    /* if value is not 1, turn off option */
    if (val == 1) {
        *x = 1;
    } else {
        *x = 0;
    }
    return ok;
}

/*parses a string values */
static int parse_string (char **s, char *arg, int num_arg) {
    if (s == NULL || arg == NULL || num_arg != 2) {
        return failure;
    }
  
    if (strncmp(arg, NULL_KEYWORD, strlen(NULL_KEYWORD)) == 0) {
        *s = NULL;
    } else {
        *s = strdup(arg); /* note: must be freed later */
    }
    return ok;
}

/* parses mutliple part string values */
static int parse_string_multiple (char **s, char *arg, int num_arg,
           unsigned int string_num, unsigned int string_num_max) {
    if (s == NULL) {
        return failure;
    }
    if (string_num >= string_num_max) {
        return failure;
    }
    return parse_string(&s[string_num], arg, num_arg);
}

/* see if parse checks are ok */
#define parse_check(s) if ((s)) {                   \
   fprintf(stderr, "error in command %s\n", command); \
   return failure;                                  \
  } else {                                          \
  return ok;                                        \
}


/* parse feature options */
static int config_parse_feature_option (struct data_feature_config *config,
                         const char *command, char *arg, int num) {  
    char *tmp;
    /* remove trailing whitespace from argument */
    tmp = arg + strnlen(arg, LINEMAX) - 1;
    while (isblank(*tmp)) {
        *tmp = 0;
        tmp--;
    }
  
    /*
     * note: because of the simplistic match function currently
     * implemented, each command name MUST NOT be a prefix of any other
     * command name; otherwise, the shorter name will be matched rather
     * than the longer one
     */
    
    if (match(command, "zeros")) {
        parse_check(parse_bool(&config->zeroes, arg, num));

    } else if (match(command, "retain")) {
        parse_check(parse_bool(&config->retains, arg, num));

    } else if (match(command, "bidir")) {
        parse_check(parse_bool(&config->bidir, arg, num));

    } else if (match(command, "dist")) {
        parse_check(parse_bool(&config->byte_distribution, arg, num));

    } else if (match(command, "entropy")) {
        parse_check(parse_bool(&config->entropy, arg, num));

    } else if (match(command, "hd")) {
        parse_check(parse_int((unsigned int*)&config->hd, arg, num, 0, HDR_DSC_LEN));

    } else if (match(command, "idp")) {
        parse_check(parse_int((unsigned int*)&config->idp, arg, num, 0, MAX_IDP));

    } else if (match(command, "exe")) {
        parse_check(parse_bool(&config->exe, arg, num));

    } else if (match(command, "dns")) {
        parse_check(parse_bool(&config->dns, arg, num));

    } else if (match(command, "ssh")) {
        parse_check(parse_bool(&config->ssh, arg, num));

    } else if (match(command, "tls")) {
        parse_check(parse_bool(&config->tls, arg, num));

    } else if (match(command, "dhcp")) {
        parse_check(parse_bool(&config->dhcp, arg, num));

    } else if (match(command, "http")) {
        parse_check(parse_bool(&config->http, arg, num));

    } else if (match(command, "ppi")) {
        parse_check(parse_bool(&config->ppi, arg, num));

    } else if (match(command, "payload")) {
        parse_check(parse_bool(&config->payload, arg, num));

    } 
    
    return ok;
}

/**
 * \fn void config_set_defaults (configuration_t *config)
 *
 * \brief Using the global \p config struct, assign the default
 *        values for options contained within.
 *
 * \param config pointer to configuration structure
 * \return none
 */
static void config_set_defaults (fnet_configuration_t *config) {

    memset(config, 0x00, sizeof (fnet_configuration_t));
    config->logfile = "stderr";
    config->verbosity = 4;
    config->show_config = 0;
    config->show_interface = 0;

    // for feature extraction default configuration    
    struct data_feature_config *dfc = &config->data_feature_cfg;
    dfc->num_pkts = 50;
    dfc->inact_timeout = 5;
    dfc->idp = 1400;
}

#define MAX_FILEPATH 128

static FILE* open_config_file(const char *filename) {
    FILE *fp = NULL;

    /* Try the filename that was given (it may be whole path needed) */
    fp = fopen(filename, "r");

#ifdef WIN32
    if (!fp) {
        /* In case of Windows install, try looking in the LocalAppData */
        char *filepath = NULL;
        PWSTR windir = NULL;

        /* Allocate memory to store constructed file path */
        filepath = calloc(MAX_FILEPATH, sizeof(char));
	if (!filepath) {
	    joy_log_err("out of memory");
	}

        SHGetKnownFolderPath(&FOLDERID_LocalAppData, 0, NULL, &windir);

        snprintf(filepath, MAX_FILEPATH, "%ls\\Joy\\%s", windir, filename);
        fp = fopen(filepath, "r");

        if (windir != NULL) {
            CoTaskMemFree(windir);
        }

        if (filepath) {
            free(filepath);
        }
    }
#endif

    if (!fp) {
        err_msg("could not open %s", filename);
    }

    return fp;
}

/**
 * \fn void config_set_from_file (configuration_t *config, const char *fname)
 *
 * \brief Read in a .cfg file and parse the contents for option values.
 *
 * \param config pointer to configuration structure
 * \param fname file with configuration items in it
 * \return ok
 * \return failure
 */
int config_data_feature_from_file (struct data_feature_config *config, const char *fname) {
    FILE *f;
    char *line = NULL;
    size_t ignore;
    int len;
    unsigned int linecount = 0;
    char *c;

    f = open_config_file(fname);
    if (f == NULL) {
        err_msg("could not find config file %s\n", fname);
        return failure;
    } 

    /*
     * Setting the default configuration values!
     */
    config_set_defaults(config);

    while ((len = getline(&line, &ignore, f)) != -1) {
        int num;
        char lhs[LINEMAX], rhs[LINEMAX];

        linecount++;
        if (len > LINEMAX) {
            fprintf(stderr, "error: line too long in file %s\n", fname);
            fclose(f);
            return failure;
        }

        /* ignore blank lines and comments */
        c = line;
        while (isblank(*c)) {
            c++;
        }
        if (*c == '#' || *c == '\n') {
            ;
        } else {
            /*
             * a valid command line consists of a LHS, possibly followed by
             * an "=" and a RHS.  The newline and # (start of comment) is
             * not part of the RHS.
             */
            num = sscanf(line, "%[^=] = %[^\n#]", lhs, rhs);
            if (num == 2 || num == 1) {
                       // printf("%s = %s ### %d ### %s", lhs, rhs, num, line);
                       
                       if (config_parse_feature_option(config, lhs, rhs, num) != ok) {
                           fprintf(stderr, "error: unknown command (%s)\n", lhs);
                           fclose(f);
                           exit(EXIT_FAILURE);
                       }
            } else if (num == 1) {
                       printf("error: could not parse line %u in file %s (\"%s ...\")\n", 
                                      linecount, fname, lhs);
                       fclose(f);
                       exit(EXIT_FAILURE);
            } else {
                       printf("error: could not parse line %s in file %s\n", line, fname);
                       fprintf(stderr, "error: could not parse line %s in file %s\n", 
                                             line, fname);
            }
        }
    }
    free(line);
    fclose(f);
    return ok;
}


/**
 * \fn int config_from_xml(configuration_t *config, const char *fname)
 *
 * \brief Read in a xml file and parse the contents for system configuration information.
 *
 * \param config pointer to configuration structure
 * \param fname file with configuration items in it
 * \return ok
 * \return failure
 */
int config_from_xml(fnet_configuration_t *config, const char *fname){
    xmlDocPtr doc = NULL;
    xmlNodePtr node= NULL;

    config_set_defaults(config);
    if((doc = xmlParseFile(fname)) == NULL){
        err_quit("parser %s error", fname);
    }

    if((node = xmlDocGetRootElement(doc)) == NULL){
        xmlFreeDoc(doc);
        err_quit("% has no element", fname);
    }
    // check root node name
    if(xmlStrcmp(node->name, BAD_CAST "Server")){
        err_quit("xml node error");
    }

    node = node->children;
    while(node != NULL){
        if(!xmlStrcmp(node->name, BAD_CAST "ConnectService")) // parse connection service configuration
        {
            if(xmlHasProp(node, BAD_CAST "address")){
                config->connect_s_cfg.address = xmlGetProp(node, BAD_CAST "address");
                puts(xmlGetProp(node, BAD_CAST "address"));
            }
            if(xmlHasProp(node, BAD_CAST "port")){
                parse_shortval(&(config->connect_s_cfg.port), xmlGetProp(node, BAD_CAST "port"), 0, 65535);
            }

        } else if(!xmlStrcmp(node->name, BAD_CAST "DistributeService")) // parse distribution service configuration
        {
            if(xmlHasProp(node, BAD_CAST "address")){
                config->distribute_s_cfg.address = xmlGetProp(node, BAD_CAST "address");
            }
            if(xmlHasProp(node, BAD_CAST "port")){
                parse_shortval(&(config->distribute_s_cfg.port), xmlGetProp(node, BAD_CAST "port"), 0, 65535);
            }
             
        } else if(!xmlStrcmp(node->name, BAD_CAST "PcapHandler"))    // parse pcap handler configuration 
        {
            
        } else if(!xmlStrcmp(node->name, BAD_CAST "Configuration")) // parse system configuration
        {
            xmlNodePtr child_node;
            // parser child node of configuration
            child_node = node->children;
            while(child_node != NULL){
                if(!xmlStrcmp(child_node->name, BAD_CAST "FlowConfig"))
                {
                    if(xmlHasProp(child_node, BAD_CAST "path")){
                        config_data_feature_from_file(&config->data_feature_cfg, xmlGetProp(child_node, BAD_CAST "path"));
                    }
                } else if(!xmlStrcmp(child_node->name, BAD_CAST "Logfile"))
                {
                    config->logfile = xmlGetProp(child_node, BAD_CAST "path");

                } else if(!xmlStrcmp(child_node->name, BAD_CAST "Interface")) 
                {
                    config->interface = xmlNodeGetContent(child_node);

                } else if(!xmlStrcmp(child_node->name, BAD_CAST "Verbosity"))
                {
                    parse_octval(&config->verbosity, xmlNodeGetContent(child_node), 0, 5); // igonre parse error

                } else if(!xmlStrcmp(child_node->name, BAD_CAST "ShowConfig"))
                {
                    parse_boolval(&config->show_config, xmlNodeGetContent(child_node)); // ignore parse error

                } else if(!xmlStrcmp(child_node->name, BAD_CAST "ShowInterface"))
                {
                    parse_boolval(&config->show_interface, xmlNodeGetContent(child_node)); // ignore parse error

                } else // ignore 
                {
                    ;
                }
                child_node = child_node->next;
            }
        }
        node = node->next;
    }
    return 0;
}

/** determine if we have avlue or not */
#define val(x) x ? x : NULL_KEYWORD 

/**
 * \fn void config_print (FILE *f, const configuration_t *c)
 * \param f file to print configuration to
 * \param c pointer to the configuration structure
 * \return none
 */
void fnet_config_print (FILE *f, const fnet_configuration_t *c) {
   
    fprintf(f, "Configurations\n==============\n");
    fprintf(f, "interface = %s\n", val(c->interface));
    fprintf(f, "logfile = %s\n", val(c->logfile));
    fprintf(f, "verbosity = %u\n", c->verbosity);
    fprintf(f, "show config = %u\n", c->show_config); 
    fprintf(f, "show interface = %u\n", c->show_interface);

    fprintf(f, "\nconnection service:\n");
    fprintf(f, "    address: %s\n",val(c->connect_s_cfg.address));
    fprintf(f, "    port: %u\n",c->connect_s_cfg.port);

    fprintf(f, "\ndistribution service:\n");
    fprintf(f, "    address: %s\n",val(c->distribute_s_cfg.address));
    fprintf(f, "    port: %u\n",c->distribute_s_cfg.port);


    

    // data feature option configuration
    const struct data_feature_config * fc = &c->data_feature_cfg;
    fprintf(f, "\ndata feature options:\n");
    fprintf(f, "    bidir = %u\n", fc->bidir);
    fprintf(f, "    zeros = %u\n", fc->zeroes);
    fprintf(f, "    retains = %u\n", fc->retains);
    fprintf(f, "    dist = %u\n", fc->byte_distribution);
    fprintf(f, "    entropy = %u\n", fc->entropy);
    fprintf(f, "    hd = %u\n", fc->hd);
    fprintf(f, "    idp = %u\n", fc->idp);

    fprintf(f, "    dns = %u\n", fc->dns);
    fprintf(f, "    ssh = %u\n", fc->ssh);
    fprintf(f, "    tls = %u\n", fc->tls);
    fprintf(f, "    dhcp = %u\n", fc->dhcp);
    fprintf(f, "    http = %u\n", fc->http);
    fprintf(f, "    ppi = %u\n", fc->ppi);
    fprintf(f, "    payload = %u\n", fc->payload);
}

