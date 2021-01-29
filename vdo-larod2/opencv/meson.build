opencv_ex_src = ['argparse.c', 'main.cc', 'misc.cc']

opencv_dep = dependency('opencv', required : true)

executable('larod-opencv-example',
           opencv_ex_src,
           c_args : [c_compiler_args],
           cpp_args : [cpp_compiler_args],
           include_directories : [inc_dir_lib,
                                  inc_dir_common],
           link_with : [larod_lib,
                        larod_log_lib,
                        utils_common_lib],
           dependencies : [opencv_dep],
           install : true)
