#!/bin/bash
#ADA 2024-25
#Validación del archivo de la práctica 6.1 que se pretende entregar
#       USAGE: $0 tar-gz-file [-h] [--debug] [--quiet] [-d dir_home]

# BITS DEL CODIGO DE RETORNO: _ _ _ _
#                             | | | |
#                             | | | -> No se admite la entrega ( o fichero inexistente)
#                             | | -> No gestiona bien los errores en los argumentos
#                             | -> No hace maze_memo()
#                             -> No hace maze_naive() (en esta práctica, esto no se permite -entrega no válida-)
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
num_practica="6.1"
source="maze.cc"
exe="maze"
err_file="errors_pr61.log"
warn_file="warnings_pr61.log"
in_tgz_must_be_files="${source} makefile"
allowed_files="${in_tgz_must_be_files} ${err_file} ${warn_file}"
make_must_build="${exe}"
should_be_func=(maze_memo maze_naive) 


#Control variables
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

[[ $quiet -eq 0 ]] && echo "Entrega de la práctica "$num_practica
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


# Check that requested functions are defined
check_num=8
for ((i=0; i<${#should_be_func[@]}; i++)); do
    grep -q ${should_be_func[$i]} $source
    if [ $? -ne 0 ]; then    
        RETURN_CODE=$((RETURN_CODE |= $(echo 2^$((i+2))|bc)))
        if [[ $(( RETURN_CODE & 8 )) -eq 8 ]]; then
            echo "Error: maze_naive es obligatoria." >> ${err_file}
            RETURN_CODE=$((RETURN_CODE |= 1))
        else
            warnings=1
            echo "Aviso: ${should_be_func[$i]} no encontrada; se asume que no se ha implementado."  >> ${warn_file}
        fi
    fi
done 

# creating test-mazes
test_1=test1.maze  #se trata de 02.maze con la casilla (2,1) cambiada
cat << 'EOF' > $test_1
6 5
1 1 0 0 1
0 1 1 1 1
0 1 1 0 0
1 1 0 1 1
1 1 1 0 0
0 0 0 1 1
EOF

cat << 'EOF' > ${test_1}.path
*1001
0*111
0*100
1*011
11*00
000**
EOF

test_2=test2.maze #se trata de 01.maze
cat << 'EOF' > $test_2
1 1
1
EOF

test_3=test3.maze #se trata de 03.maze; sin salida
cat << 'EOF' > $test_3
6 5
1 1 0 0 1
1 1 1 1 1
0 1 0 0 0
1 0 0 1 1
1 0 1 0 0
0 0 0 1 1
EOF

test_4=test4.maze
cat << 'EOF' > $test_4
10 10
1 1 1 1 1 1 1 1 1 1
1 1 1 1 1 1 1 1 1 1
1 1 1 1 1 1 1 1 1 1
1 1 1 1 1 1 1 1 1 1
1 1 1 1 1 1 1 1 1 1
1 1 1 1 1 1 1 1 1 1
1 1 1 1 1 1 1 1 1 1
1 1 1 1 1 1 1 1 1 1
1 1 1 1 1 1 1 1 1 1
1 1 1 1 1 1 1 1 1 1
EOF


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
command="$TIMEOUT_0 ${exe}  -f $test_1 -t -mal1 -mal2"
command2show="${exe} -f 02.maze -t -mal1 -mal2"
check_err_args "$command" "$command2show" "$check_num (ERROR)"

check_num=16
command="$TIMEOUT_0 ${exe} -f $test_1 -f inexistentefile -t"
command2show="${exe} -f 02.maze -f inexistentefile -t"
check_err_args "$command" "$command2show" "$check_num (ERROR)"


# checks on solutions and output-format

check_num=17
command="$TIMEOUT_0 ${exe} -f $test_3 --ignore-naive"
command2show="${exe} -f 03.maze --ignore-naive"
launch "$command" "$command2show" $check_num
if [[ ! $(echo "$captured" | wc -l) -eq 1  ]]; then
    [[ -z "$captured" ]] && captured="<void>"
    error_msg "salida capturada:\n$captured\n La salida debe estar compuesta de una sola línea." "$command2show" "$check_num (ERROR)"
fi

check_num=18
command="$TIMEOUT_0 ${exe} -f $test_1 --ignore-naive"
command2show="${exe} -f 02.maze --ignore-naive"
launch "$command" "$command2show" $check_num
if [[ ! $(echo $captured | head -n 1 | grep -Eo '7|\?' | wc -l) -eq 3  ]]; then
    [[ -z "$captured" ]] && captured="<void>"
    error_msg "salida capturada:\n$captured" "$command2show" "$check_num (ERROR)"
fi

check_num=19
command="$TIMEOUT_0 ${exe} -f $test_3 --ignore-naive --p2D"
command2show="${exe} -f 03.maze --ignore-naive --p2D"
launch "$command" "$command2show" $check_num
if [[ ! $(echo "$captured" | wc -l) -eq 2  ]]; then
    [[ -z "$captured" ]] && captured="<void>"
    error_msg "salida capturada:\n$captured\n La salida debe estar compuesta solo de 2 líneas." "$command2show" "$check_num (ERROR)"
fi


check_num=20
command="$TIMEOUT_0 ${exe} -f $test_3 --ignore-naive"
command2show="${exe} -f 03.maze --ignore-naive"
launch "$command" "$command2show" $check_num
if [[ ! $(echo $captured | head -n 1 | grep -Eo '0|\?' | wc -l) -eq 3  ]]; then
    [[ -z "$captured" ]] && captured="<void>"
    error_msg "salida capturada:\n$captured" "$command2show" "$check_num (ERROR)"
fi

check_num=21
command="$TIMEOUT_0 ${exe} -f $test_4 --ignore-naive"
command2show="${exe} -f 03bis.maze --ignore-naive (laberinto 10x10 todo a unos)"
launch "$command" "$command2show" $check_num
if [[ ! $(echo $captured | head -n 1 | grep -Eo '10|\?' | wc -l) -eq 3  ]]; then
    [[ -z "$captured" ]] && captured="<void>"
    error_msg "salida capturada:\n$captured" "$command2show" "$check_num (ERROR)"
fi


check_num=22
command="$TIMEOUT_0 ${exe} -f $test_1"
command2show="${exe} -f 02.maze"
launch "$command" "$command2show" $check_num
if [[ ! $(echo $captured | grep -Eo '7|\?' | wc -l) -eq 4  ]]; then
    [[ -z "$captured" ]] && captured="<void>"
    error_msg "salida capturada:\n$captured" "$command2show" "$check_num (ERROR)"
fi


check_num=23
command="$TIMEOUT_0 ${exe} -f $test_1"
command2show="${exe} -f 02.maze"
launch "$command" "$command2show" $check_num
if [[ ! $(echo $captured | wc -w) -eq 4  ]]; then
    [[ -z "$captured" ]] && captured="<void>"
    error_msg "salida capturada:\n$captured" "$command2show" "$check_num (ERROR)"
fi


check_num=24
command="$TIMEOUT_0 ${exe} --ignore-naive -f $test_1"
command2show="${exe} --ignore-naive -f 02.maze"
launch "$command" "$command2show" $check_num
if [[ ! "$captured" =~ ^[[:space:]]*-[[:space:]] ]]; then
    [[ -z "$captured" ]] && captured="<void>"
    error_msg "salida capturada:\n$captured" "$command2show" "$check_num (ERROR)"
fi

msg_bad_output_tables="  En la posición de los almacenes, se espera el carácter '?' o ambos almacenes (en el formato correcto)"
check_num=25
command="$TIMEOUT_0 ${exe} -f $test_2 -t"
command2show="${exe} -f 01.maze -t"
launch "$command" "$command2show" $check_num
if [[ ! $(echo "$captured" | tail -n +2 ) == "?"  ]] &&
   [[ ! $(echo $captured | grep -Eo '1|\?' | wc -l) -eq 6  ]]; then
    [[ -z "$captured" ]] && captured="<void>"
    error_msg "salida capturada:\n$captured \n $msg_bad_output_tables" "$command2show" "$check_num (ERROR)"
fi

check_num=26
msg_bad_output_header="  El/los encabezado/s de la tabla de almacén no es correcto"
command="$TIMEOUT_0 ${exe} -f $test_2 -t"
command2show="${exe} -f 01.maze -t"
launch "$command" "$command2show" $check_num
if [[ ! $(echo "$captured" | tail -n +2 ) == "?"  ]] &&
   [[ ! $(echo $captured | grep -Eo 'Memoization table|Iterative table' | wc -l) -eq 2  ]]; then
    [[ -z "$captured" ]] && captured="<void>"
    error_msg "salida capturada:\n$captured \n $msg_bad_output_header" "$command2show" "$check_num (ERROR)"
fi

param_path="--p2D"
msg_bad_output_path="  En la posición del camino, se espera el carácter '?' o el camino correcto (en el formato correcto)"
check_num=27
command="$TIMEOUT_0 ${exe} ${param_path} -f $test_2 --ignore-naive"
command2show="${exe} ${param_path} -f 01.maze --ignore-naive"
launch "$command" "$command2show" $check_num
captured=$(echo "$captured" | head -n 2)
select=$(echo "$captured" | tr -d ' \n'| tail -c 1)
if [[ ! ( "$select" == "?" || "$select" == "*" ) ]] ; then
    [[ -z "$captured" ]] && captured="<void>"
    error_msg "salida capturada:\n$captured \n $msg_bad_output_path" "$command2show" "$check_num (ERROR)"
fi  

check_num=28
command="$TIMEOUT_0 ${exe} ${param_path} -f $test_2 --ignore-naive -t"
command2show="${exe} ${param_path} -f 01.maze --ignore-naive -t"
launch "$command" "$command2show" $check_num
error=1
for i in \*Memoizationtable:1Iterativetable:1 ?Memoizationtable:1Iterativetable:1 \*? ?? ; do   #secuencias válidas
   [[ "$(echo "$captured" | tail -n +2 | tr -d ' \n')" == "$i" ]] && { error=0; break; }
done
if [[ $error -eq 1 ]]; then
    [[ -z "$captured" ]] && captured="<void>"
    error_msg "salida capturada:\n$captured \n $msg_bad_output_tables" "$command2show" "$check_num (ERROR)"
fi

check_num=29
command="$TIMEOUT_0 ${exe} ${param_path} -f $test_1 --ignore-naive"
command2show="${exe} ${param_path} -f 02.maze --ignore-naive"
launch "$command" "$command2show" $check_num
captured=$(echo "$captured"  | tail -n +2 | head -n 7 )
if [[ $(echo $captured | tr -d ' ') != "?" ]] && [[ $captured != "$(cat ${test_1}.path)" ]]; then
    [[ -z "$captured" ]] && captured="<void>"
    error_msg "salida capturada:\n$captured \n $msg_bad_output_path" "$command2show" "$check_num (ERROR)"
fi


( [[ -f ${err_file} ]] || [[ $((RETURN_CODE&1)) -eq 1 ]] ) && exit 1;
exit 0



