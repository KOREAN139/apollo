#! /usr/bin/env bash
#### SHOULD BE RUN INSIDE THE DOCKER!!! ####

MODULE_PATH="/apollo/modules"

function main() {
    # parse_cmdline_args "$@"
    gcda_list=$(find /apollo/cyber/bazel-out -name "*gcda")
    gcda_array=($gcda_list)

    gcno_list=$(find /apollo/.cache -name "*gcno")
    gcno_array=($gcno_list)

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
        fi
    done

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

    return
}

main "$@"
