option(GGML_NATIVE
       "Enable -mcpu=native for host CPU (set OFF when cross-compiling)"
       $<$<NOT:$<BOOL:${CROSS_COMPILE}>>:ON>) 


