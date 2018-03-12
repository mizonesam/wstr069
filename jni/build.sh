build_type=${1#*+}
cur_path=${1%+*}

if [ -z $cur_path ]; then
    echo ">>> Invalid PATH <<<"
    exit 1
fi

if [ -z $build_type ]; then
    echo ">>> Invalid MODE <<<"
    exit 1
fi

#echo $cur_path $build_type
timestamp=`date "+%Y-%m-%d,%H:%M:%S"`
revise=`   svn info $cur_path | grep "Revision" | cut -d' ' -f2`
repo_path=`svn info $cur_path | grep "Relative" | cut -d' ' -f3`
_uuid=`    svn info $cur_path | grep UUID       | cut -d' ' -f3`
repo_user=`svn auth           | grep -A3 $_uuid | grep Username | cut -d' ' -f2`

cat<<EOF>$cur_path/version.h
#ifndef _VERSION_H_
#define _VERSION_H_

#define VERSION    "$revise"
#define REPO_USER  "$repo_user"
#define TIMESTAMP  "$timestamp"
#define BUILD_TYPE "$build_type"
#define REPO_PATH  "$repo_path"

#define VER_STR \\
                "Version: "      VERSION",\n"   \\
                "Who compiled: " REPO_USER",\n" \\
                "Timestamp: "    TIMESTAMP",\n" \\
                "Build type: "   BUILD_TYPE",\n"\\
                "Relative path: "REPO_PATH".\n" \\

#endif /* _VERSION_H_ */
EOF

#echo "#ifndef _VERSION_H_"  > $cur_path/ver.h
#echo "#define _VERSION_H_" >> $cur_path/ver.h
#echo -e "\x0D"             >> $cur_path/ver.h

#echo "#define VERSION    \"$revise\""    >>$cur_path/ver.h
#echo "#define REPO_USER  \"$repo_user\"" >>$cur_path/ver.h
#echo "#define VERSION    \"$timestamp\"" >>$cur_path/ver.h
#echo "#define BUILD_TYPE \"$build_type\"">>$cur_path/ver.h
#echo "#define REPOPATH   \"$repo_path\"" >>$cur_path/ver.h

#echo -e "\x0D"                        >> $cur_path/ver.h
#echo "extern const char* ver = \"Version: \"      VERSION\",\\n\"">>    $cur_path/ver.h
#echo "                         \"Who compiled: \" REPO_USER\",\\n\"">>  $cur_path/ver.h
#echo "                         \"Timestamp: \"    TIMESTAMP\",\\n\"">>  $cur_path/ver.h
#echo "                         \"Build type: \"   BUILD_TYPE\",\\n\"">> $cur_path/ver.h
#echo "                         \"Relative path: \"REPO_PATH\".\\n\";">> $cur_path/ver.h

#echo -e "\x0D"                        >> $cur_path/ver.h
#echo  "#endif /* _VERSION_H_ */"      >> $cur_path/ver.h
