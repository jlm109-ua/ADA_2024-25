#!/bin/bash
#ADA 2024-25
#Validación del archivo de la práctica 7 que se pretende entregar
#       USAGE: $0 tar-gz-file [-h] [--debug] [--quiet] [-d dir_home]

# BITS DEL CODIGO DE RETORNO: _ _ _
#                             | | |
#                             | | -> No se admite la entrega ( o fichero inexistente)
#                             | -> No gestiona bien los errores en los argumentos
#                             -> No hace mcp_greedy() (en esta práctica, esto no se permite -entrega no válida-)
export PATH=.:$PATH


#####  GESTION DE LA ORDEN 'timeout' ######################
TIMEOUT_0="timeout 0.5"

which timeout &> /dev/null
if [[ $? -ne 0 ]]; then
    echo
    echo "ATENCIÓN: No se detecta la orden 'timeout' en tu sistema. La validación se tendrá que realizar sin hacer uso de esa orden, pero ten en cuenta que en la corrección sí se hará uso de ella."
    echo
    echo "(con las pruebas que se hacen en esta validación, no se permitirá que el programa produzca violaciones de segmento o que tarde demasiado. Si ocurre alguna de estas circunstancias se asumirá que la validación es NEGATIVA. La orden 'timeout' se utiliza para detectar esos casos.)"
    echo
    TIMEOUT_0=""
fi


# variables to be assigned in each job
num_practica="7"
source="maze_greedy.cc"
exe="maze_greedy"
err_file="errors_pr7.log"
warn_file="warnings_pr7.log"
in_tgz_must_be_files="${source} makefile"
allowed_files="${in_tgz_must_be_files} ${err_file} ${warn_file}"
make_must_build="${exe}"
should_be_func=(maze_greedy) 


#Control variables
[ ! -z ${TIMEOUT} ] && { TIMEOUT_0="${TIMEOUT} 0.5"; } || TIMEOUT_0="" 
RETURN_CODE=0
allowed_ext=".tar.gz .tgz"
warnings=0
name="<unknown>"
captured=""
msg_timeout=""
check_num=0

filename=""     #file (tgz) to validate
dir_home=$(pwd) #directory where is TEST subdirectory. Assumed current directory
dir_tgz=""      #directory where tgz is
quiet=0
debug=0

#argument management
args=( "$@" )
for ((i=0; i<${#args[@]}; i++)); do
    is_filename=1
    [[ ${args[$i]} == "--debug" ]] && { debug=1; is_filename=0; }
    [[ ${args[$i]} == "--quiet" ]] && { quiet=1; is_filename=0; }
    [[ ${args[$i]} == "-d" ]] && { i=$((i+1)); dir_home=${args[$i]}; is_filename=0; }
    [[ ${args[$i]} == "-h" ]] && { echo "help: $0 tar.gz-file [-h] [--debug] [--quiet] [-d dir_home]";  exit 255; }
    [[ is_filename -eq 1 ]] && filename="${args[$i]}"
done

[[ $quiet -eq 0 ]] && echo "Entrega de la práctica "${num_practica}
[[ $quiet -eq 0 ]] && echo "Fichero a validar: ${filename}"

[   -z "$filename" ] && { echo $0: missing tar.gz file; exit 255; }
[ ! -f "$filename" ] && { echo $0: file \'$filename\' not found; exit 255; }



#Create tmp directory
tmp_dir=$(mktemp -d /tmp/validate_dir.XXXXXX)

# Actions at the end
by_the_end() {
    exit_code=$?
    [[ -f ${err_file} ]] && exit_code=1
    [[ -f ${warn_file} ]] && warnings=1
    [[ $exit_code -ne 0 ]] && RETURN_CODE=$((RETURN_CODE |= 1))
    if [[ $quiet -eq 0 ]]; then
        [[ -f ${warn_file} ]] && { echo; cat ${warn_file}; }
        [[ $warnings  -ne 0 ]] && echo -e "\nATENCIÓN: Los avisos mostrados pueden mermar considerablemente la nota."
        [[ -f ${err_file} ]] && { echo; cat ${err_file}; }
        if [[ $exit_code -eq 0 ]]; then
            echo -e "\nDNI/NIE asociado al archivo comprimido: "$dni
            echo -e "Nombre capturado en la primera línea de ${source}: "$name
            echo -e "Asegúrate de que la captura de DNI/NIE y nombre es correcta."
            echo -e "\nVALIDACIÓN POSITIVA: El trabajo cumple las especificaciones, pero asegúrate de que también compila en los ordenadores del aula."
        else
            echo -e "\nVALIDACIÓN NEGATIVA: Debes corregir los errores antes de entregar."
        fi
#    else
#        echo -n $RETURN_CODE
    fi
    rm -r $tmp_dir
    exit $RETURN_CODE
}

error_msg(){
    echo -e "\n*** Validación número $3 ***" >> ${err_file}
    echo -e "  Prueba realizada: $2" >> ${err_file}
    echo -e "  $1" >> ${err_file}
    RETURN_CODE=$(( RETURN_CODE |= 1 ))
}


check_err_args() {
#No se admite violación de segmento, core, etc.
    error_message=$($1 2>&1 1>/dev/null)
    ret=$? # 124=timeout; 134=seg-fault; 139=core-dumped
    if [[ ret -eq 124 ]] || [[ ret -eq 134 ]] || [[ ret -eq 139 ]]; then
        error_msg "Error: La orden produce violación de segmento o se cuelga" "$2" "$3"
#        echo "Aviso: '$2' produce violación de segmento o 'timeout'" >> ${warn_file}
#        warnings=1
    fi
    if [[ -z "$error_message" ]]; then
        echo "Aviso: '$2' no emite mensaje de error (por la salida de error)"  >> ${warn_file}
        warnings=1
    fi 
    [[ warnings -eq 1 ]] &&  RETURN_CODE=$(( RETURN_CODE |= 2 ))
}

check_code() {
    gprof -p -b $1 gmon.out | grep $2
    if [ $? -ne 0 ]; then
        check_call_funct $2
    fi
    check_memory_maze --save $1 $2    
}

launch() {
    [[ -f tmp_file ]] && rm tmp_file
    captured=$($1 2>tmp_file); ret=$?
    case $ret in
	    124) msg_timeout="tarda demasiado ('timeout').";;
	    134) msg_timeout="produce violación de segmento.";;
	    139) msg_timeout="produce un 'core'.";;
	    *)   [[ -s tmp_file ]] && { msg_timeout="no debe mostrar nada por la salida de error puesto que es correcta."; ret=1; } || ret=0
    esac
    [[ $ret -ne 0 ]] && error_msg "Error: la orden ${msg_timeout}" "$2" $3
    return $ret
}

function validate_n_nums() {
# códigos de error: 
#   1:más elementos de lo especificado  $2; 
#   2:alguno de los elementos no son números
captured_array=()
read -a captured_array < <(echo $1)
[[ ${#captured_array[@]} -ne $2 ]] && return 1
d=[0-9]
for ((i=0; i<${#captured_array[@]}; i++)); do
    [[ ! ${captured_array[i]} =~ ^$d+([.]$d+)?([Ee][-+]?$d+)?$ ]] && return 2
done
return 0
}


#capture signals
trap by_the_end EXIT SIGINT SIGTERM


# Work in tmp dir; capture tgz name and directory where it is
cp "$filename" $tmp_dir
cd "$(dirname "${filename}")"
dir_tgz=$(pwd)
filename="$(basename "${filename}")" #filename=${filename##*/}
cd $tmp_dir



# dni/nie & extension of the file to be delivered
check_num=1
dni=$(echo "$filename" | cut -d "." -f 1)
ext=.$(echo "$filename" | cut -d "." -f 2-3) # | tr -d '\n' | tail -c 2)
if [[ ! " $allowed_ext " =~ " $ext " ]];then
    echo "Error: Las extensiones permitidas son: {"$allowed_ext"}."  >> ${err_file}
    echo -e "\t(extensión capturada: $ext )" >> ${err_file}
    RETURN_CODE=$((RETURN_CODE |= 1))
    exit 1
fi

# unpack tgz
check_num=2
tar -xzvf "$filename" > /dev/null
if [ $? -ne 0 ]; then
    echo "Error: 'tar' falló al desempaquetar ${filename}." >> ${err_file}
    RETURN_CODE=$((RETURN_CODE |= 1))
    exit 1
fi

# keep only what is stored in the tgz
rm "$filename"
[[ -f ${err_file}  ]] && rm ${err_file}
[[ -f ${warn_file} ]] && rm ${warn_file}


# Verify that only has been delivered what is requested
check_num=3
missing=""
for f in ${in_tgz_must_be_files}; do
    if [ ! -f $f ]; then
        missing=${missing}" "${f}
    fi 
done
if [[ ! -z ${missing} ]]; then
    echo "Error: Falta/n archivo/s en la entrega (${missing} )." >> ${err_file}
    RETURN_CODE=$((RETURN_CODE |= 1))
fi


# verify that only what is necessary is delivered
check_num=4
unwanted=""
for f in *; do
    if [[ ! " $allowed_files " =~ " $f " ]];then 
        unwanted=${unwanted}" "${f}
    fi
done
if [[ ! -z ${unwanted} ]]; then
    echo "Error: Se entregan más archivos de los permitidos (${unwanted} )." >> ${err_file}
    RETURN_CODE=$((RETURN_CODE |= 1))
fi


#name="$(echo -e "${name}" | sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//')" # All-TRIM
#tr -d "[:space:]" #elimina tb saltos de linea, a diferencia de tr -d "" que elimina solo espacios
# Verify the student's name in the first line of the source (only first line is checked)
check_num=5
[[ ! -f ${source} ]] && { RETURN_CODE=$((RETURN_CODE |= 1)); exit 1; }
name=$(cat ${source} | head -n 1 | tr '*' '/' | xargs)
[[ "$name" =~ ^"//" ]] && name=$(echo ${name} | tr -d '/' | xargs) || name="<unknown>"
[[ -z ${name} ]] && name="<unknown>"
if [[ "$name" == "<unknown>" ]]; then
    echo "Error: No se captura tu nombre en la primera línea del archivo ${source}." >> ${err_file}
    RETURN_CODE=$((RETURN_CODE |= 1))
fi

#compilation
check_num=6
make > /dev/null
if [ $? -ne 0 ]; then
    echo "Error: 'make' ha devuelto error." >> ${err_file}
    RETURN_CODE=$((RETURN_CODE |= 1))
    exit 1
fi


# Verify that the executable file exists
check_num=7
if [ ! -x "$exe" ]; then
    echo "Error: El archivo ${exe} no existe o no es ejecutable." >> ${err_file}
    RETURN_CODE=$((RETURN_CODE |= 1))
    exit 1
fi


# Check that the requested functions are defined
check_num=8
for ((i=0; i<${#should_be_func[@]}; i++)); do
    grep -q ${should_be_func[$i]} $source
    if [ $? -ne 0 ]; then    
        RETURN_CODE=$((RETURN_CODE |= $(echo 2^$((i+2))|bc)))
        if [[ $(( RETURN_CODE & 4 )) -eq 4 ]]; then
            echo "Error: La función (o método de clase) 'maze_greedy()' es obligatoria." >> ${err_file}
            RETURN_CODE=$((RETURN_CODE |= 1))
            exit 1
        else
            warnings=1
            echo "Aviso: ${should_be_func[$i]} no encontrada; se asume que no se ha implementado."  >> ${warn_file}
        fi
    fi
done 

# creating test-mazes
cat << 'EOF' > 00.maze
1 1
0
EOF

cat << 'EOF' > 01.maze
1 1
1
EOF

cat << 'EOF' > 01.maze.sol_greedy
1
*
EOF


cat << 'EOF' > 02.maze  
6 5
1 1 0 0 1
1 1 1 1 1
0 1 1 0 0
1 1 0 1 1
1 1 1 0 0
0 0 0 1 1
EOF

cat << 'EOF' > 02.maze.sol_greedy
0
*1001
1*111
01*00
110**
11100
00011
EOF


cat << 'EOF' > 04.maze
3 1
1 
1
1
EOF

cat << 'EOF' > 05.maze
1 3 
1 1 1
EOF

cat << 'EOF' > 06.maze
3 3
0 1 1 
1 1 1
1 1 1
EOF

cat << 'EOF' > 07.maze
10 8
1 1 0 0 1 1 1 1  
1 1 1 1 1 1 1 1  
0 1 1 0 0 1 1 1  
1 1 0 0 1 0 1 1  
1 1 1 0 0 0 1 1  
0 0 0 0 0 0 1 1  
1 1 1 1 1 1 1 1  
1 1 0 0 0 1 1 1  
1 1 1 1 0 0 1 0  
1 1 1 1 1 0 0 1 
EOF

cat << 'EOF' > 08.maze
20 20
1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 
1 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 
1 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 0 
1 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 0 
1 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 0 
1 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 0 
1 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 0 
1 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 0 
1 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 0 
1 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 0 
1 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 0 
1 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 0 
1 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 0 
1 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 0 
1 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 0 
1 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 0 
1 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 0 
1 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 0 
1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 
1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1
EOF


cat << 'EOF' > 09.maze
25 25
1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 
0 1 1 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 1 
0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 1 
0 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 1 
0 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 1 
0 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 1 
0 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 1 
0 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 1 
0 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 1 
0 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 1 
0 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 1 
0 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 1 
0 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 1 
0 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 1 
0 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 1 
0 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 1 
0 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 1 
0 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 1 
0 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 1 
0 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 1 
0 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 1 
0 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 1 
0 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 1
0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 1 0 1
0 0 0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 1 1 1
EOF

cat << 'EOF' > 11.maze
6 6
1 0 1 1 1 1
1 0 1 1 1 1
1 0 0 0 0 1
1 1 1 1 1 1
1 1 1 1 1 1
1 1 1 1 1 1
EOF

cat << 'EOF' > 11.maze.sol_greedy
8
*01111
*01111
*00001
1*1111
11*111
111***
EOF

echo 1001 40 > 1001x40.maze
l="1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1"
for ((i=0; i<1001; i++)); do
    echo $l >> 1001x40.maze
done

# Command line checks. 
# Ante cualquier aviso, se considerará que la gestión de argumentos no es robusta
# No se permiten violaciones de segmento, cores, etc.

check_num=11
command="$TIMEOUT_0 ${exe} -f"
command2show="${exe} -f"
check_err_args "$command" "$command2show" "$check_num (ERROR)"

check_num=12
command="$TIMEOUT_0 ${exe} -f $test_1 -f"
command2show="${exe} -f 02.maze -f"
check_err_args "$command" "$command2show" "$check_num (ERROR)"

check_num=13
command="$TIMEOUT_0 ${exe}"
command2show="${exe}"
check_err_args "$command" "$command2show" "$check_num (ERROR)"

check_num=14
command="$TIMEOUT_0 ${exe} -f -// "
command2show="${exe}  -f -//"
check_err_args "$command" "$command2show" "$check_num (ERROR)"

check_num=15
command="$TIMEOUT_0 ${exe}  -f $test_1 --ignore-naive" #En este caso, la opción es incorrecta
command2show="${exe} -f 02.maze --ignore-naive"
check_err_args "$command" "$command2show" "$check_num (ERROR)"

check_num=16
command="$TIMEOUT_0 ${exe} -f $test_1 -f inexistentefile -t"
command2show="${exe} -f 02.maze -f inexistentefile -t"
check_err_args "$command" "$command2show" "$check_num (ERROR)"


# checks on solutions and output-format

check_num=17
command="$TIMEOUT_0 ${exe} -f 00.maze"
command2show="${exe} -f 00.maze"
launch "$command" "$command2show" $check_num
if [[ ! $(echo "$captured" | wc -l) -eq 1  ]]; then
    [[ -z "$captured" ]] && captured="<void>"
    error_msg "salida capturada:\n$captured\n La salida debe estar compuesta de una sola línea." "$command2show" "$check_num (ERROR)"
fi

check_num=18
command="$TIMEOUT_0 ${exe} -f 02.maze"
command2show="${exe} -f 02.maze"
launch "$command" "$command2show" $check_num
if [[ ! $(echo "$captured" | wc -l) -eq 1  ]]; then
    [[ -z "$captured" ]] && captured="<void>"
    error_msg "salida capturada:\n$captured\n La salida debe estar compuesta de una sola línea." "$command2show" "$check_num (ERROR)"
fi


check_num=19
if [[ ! $captured -eq 0  ]]; then
    [[ -z "$captured" ]] && captured="<void>"
    error_msg "salida capturada:\n$captured\n La solución es incorrecta." "$command2show" "$check_num (ERROR)"
fi

check_num=20
command="$TIMEOUT_0 ${exe} -f 04.maze"
command2show="${exe} -f 04.maze"
launch "$command" "$command2show" $check_num
if [[ ! $captured -eq 3  ]]; then
    [[ -z "$captured" ]] && captured="<void>"
    error_msg "salida capturada:\n$captured\n La solución es incorrecta." "$command2show" "$check_num (ERROR)"
fi


check_num=21
command="$TIMEOUT_0 ${exe} -f 05.maze"
command2show="${exe} -f 05.maze"
launch "$command" "$command2show" $check_num
if [[ ! $captured -eq 3  ]]; then
    [[ -z "$captured" ]] && captured="<void>"
    error_msg "salida capturada:\n$captured\n La solución es incorrecta." "$command2show" "$check_num (ERROR)"
fi


check_num=22
command="$TIMEOUT_0 ${exe} -f 06.maze"
command2show="${exe} -f 06.maze"
launch "$command" "$command2show" $check_num
if [[ ! $captured -eq 0  ]]; then
    [[ -z "$captured" ]] && captured="<void>"
    error_msg "salida capturada:\n$captured\n La solución es incorrecta." "$command2show" "$check_num (ERROR)"
fi

check_num=23
command="$TIMEOUT_0 ${exe} -f 09.maze"
command2show="${exe} -f 09.maze"
launch "$command" "$command2show" $check_num
if [[ ! $captured -eq 26  ]]; then
    [[ -z "$captured" ]] && captured="<void>"
    error_msg "salida capturada:\n$captured\n La solución es incorrecta." "$command2show" "$check_num (ERROR)"
fi


check_num=24
command="$TIMEOUT_0 ${exe} -f 11.maze"
command2show="${exe} -f 11.maze"
launch "$command" "$command2show" $check_num
if [[ ! $captured -eq 8  ]]; then
    [[ -z "$captured" ]] && captured="<void>"
    error_msg "salida capturada:\n$captured\n La solución es incorrecta." "$command2show" "$check_num (ERROR)"
fi

msg_bad_output_path="  El camino no es correcto; no corresponde a un criterio voraz o el formato no cumple las especificaciones"

check_num=25
command="$TIMEOUT_0 ${exe} --p2D -f 11.maze"
command2show="${exe} --p2D -f 11.maze"
launch "$command" "$command2show" $check_num
if [[ "$captured" != "$(cat 11.maze.sol_greedy)" ]]; then
    [[ -z "$captured" ]] && captured="<void>"
    error_msg "salida capturada:\n$captured \n $msg_bad_output_path" "$command2show" "$check_num (ERROR)"
fi

check_num=26
command="$TIMEOUT_0 ${exe} --p2D -f 02.maze"
command2show="${exe} --p2D -f 02.maze"
launch "$command" "$command2show" $check_num
if [[ "$captured" != "$(cat 02.maze.sol_greedy)" ]]; then
    [[ -z "$captured" ]] && captured="<void>"
    error_msg "salida capturada:\n$captured \n $msg_bad_output_path" "$command2show" "$check_num (ERROR)"
fi


check_num=27
command="$TIMEOUT_0 ${exe} --p2D -f 01.maze"
command2show="${exe} --p2D -f 01.maze"
launch "$command" "$command2show" $check_num
if [[ "$captured" != "$(cat 01.maze.sol_greedy)" ]]; then
    [[ -z "$captured" ]] && captured="<void>"
    error_msg "salida capturada:\n$captured \n $msg_bad_output_path" "$command2show" "$check_num (ERROR)"
fi

check_num=28
command="$TIMEOUT_0 ${exe} -f 1001x40.maze"
command2show="${exe} -f 1001x40.maze"
launch "$command" "$command2show" $check_num
if [[ ! $captured -eq 1001  ]]; then
    [[ -z "$captured" ]] && captured="<void>"
    error_msg "salida capturada:\n$captured\n La solución de un laberinto 1001x40 todo con unos." "$command2show" "$check_num (ERROR)"
fi


( [[ -f ${err_file} ]] || [[ $((RETURN_CODE&1)) -eq 1 ]] ) && exit 1;
exit 0



