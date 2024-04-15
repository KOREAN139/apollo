#! /usr/bin/env bash
#### SHOULD BE RUN INSIDE THE DOCKER!!! ####

MODULE_PATH="/apollo/modules"

function clean_gcov() {
    find . -name "*gcov" -delete
    find /apollo/modules/ -name "*gcda" -delete
}

function move_gcda() {
    gcda_list=$(find /apollo/cyber/bazel-out -name "*gcda")
    gcda_array=($gcda_list)

    for line in "${gcda_array[@]}"
    do
        set -f                     # avoid globbing (expansion of *).
        array=(${line//// })
        bool=0
        gcda_path="/apollo/"
        for tmp in "${array[@]}"
        do
            if [ $bool == 1 ]
            then
                if [ $tmp == "_objs" ]
                then
                    break
                else
                    gcda_path+=$tmp/
                fi
            elif [ $tmp == "bin" ]
            then
                bool=1
            fi
        done
        fname=(${array[-1]//./ })
        fname_wo_pic="${fname[0]}".gcda
        source_file="${fname[0]}".cc
        copied_path=$gcda_path$fname_wo_pic

        if [ -d $gcda_path ] && [ $(find $gcda_path -name $source_file | wc -l) > 0 ]
        then
            cp $line $copied_path
        else
          echo $line
        fi
    done

}

function move_gcno() {
    gcno_list=$(find /apollo/.cache -name "*gcno")
    gcno_array=($gcno_list)

    for line in "${gcno_array[@]}"
    do
        set -f                     # avoid globbing (expansion of *).
        array=(${line//// })
        bool=0
        gcno_path="/apollo/"
        for tmp in "${array[@]}"
        do
            if [ $bool == 1 ]
            then
                if [ $tmp == "_objs" ]
                then
                    break
                else
                    gcno_path+=$tmp/
                fi
            elif [ $tmp == "bin" ]
            then
                bool=1
            fi
        done
        fname=(${array[-1]//./ })
        fname_wo_pic="${fname[0]}".gcno
        source_file="${fname[0]}".cc
        copied_path=$gcno_path$fname_wo_pic

        if [ -d $gcno_path ] && [ $(find $gcno_path -name $source_file | wc -l) > 0 ]
        then
            cp $line $copied_path
        fi
    done
}

function run_gcov() {
    lcov -c -d /apollo/modules/planning/scenarios/ -o coverage.info
    lcov -r coverage.info "/apollo/external/*" -o coverage_new.info
    genhtml --output-directory output_directory coverage_new.info
}

function main() {
    # parse_cmdline_args "$@"

    clean_gcov
    
    move_gcda
    move_gcno

    run_gcov

    return
}

main "$@"
