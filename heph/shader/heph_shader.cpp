#include "heph_shader.hpp"

#include <stdint.h>
#include <string.h>
#include <stdlib.h>


#define HEPH_SHADER_PREPROCESSOR_ERRNO_UNDEFINED_DIRECTIVE 1


typedef struct 
{
        bool has_args;
        uint32_t sb;
        char text[1];
} HephShaderPreprocessMacro;

typedef struct
{
        uint32_t ndirectives;
        char *directives[1];
        HephShaderPreprocessMacro *macros;

        uint32_t nerrs, errcap;
        uint32_t *errnos;       
} HephShaderPreprocessor;


void heph_shader_preprocessor_push_error(HephShaderPreprocessor *const p, uint32_t errno)
{
        if (p->nerrs + 1 > p->errcap)
        {
                p->errnos = (uint32_t *)realloc(p->errnos, p->errcap * 2);
        }    

        p->errnos[p->nerrs++] = errno;
}

void heph_shader_preprocessor_parse_directive(HephShaderPreprocessor *const p, const char *token, uint32_t token_sb)
{
        const char *seek = token;
        while (*seek != ' ')
                ++seek;
        
        for (uint32_t i = 0; i < p->ndirectives; i++)
        {
                if (!strncmp(token, p->directives[i], seek - token))
                {
                        break;
                }
                heph_shader_preprocessor_push_error(p, HEPH_SHADER_PREPROCESSOR_ERRNO_UNDEFINED_DIRECTIVE);
        }
}

char *heph_shader_preprocess_shader(char *const src, uint32_t src_sb)
{
        char *errs = NULL;
        
        char delims[] = "\n";
        char *token = strtok(src, delims);
        while (token != NULL)
        {       
                while (*token == ' ')
                        ++token;

                if (*token == '#')
                {
                        heph_shader_preprocessor_parse_directive();
                }


                token = strtok(NULL, delims);
        }

}




