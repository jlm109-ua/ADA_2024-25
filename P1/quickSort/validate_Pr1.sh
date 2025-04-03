#!/bin/bash

#ADA (II-I2ADE - DLSI-UA) 2024-25
#Validación del archivo de la práctica 1 que se pretende entregar
#       USAGE: $0 tar-gz-file [-h] [--debug] [--quiet] [-d dir_home]

# BITS DEL CODIGO DE RETORNO:  _ _
#                              | |
#                              | -> No se admite la entrega ( o fichero inexistente)
#                              | -> Se han producido AVISOS (se permite la entrega pero mermará la nota)


export PATH=.:$PATH

#####  GESTION DE LA ORDEN 'timeout' ######################
MAX_TIME=30
TIMEOUT="timeout ${MAX_TIME}"

which timeout &> /dev/null
if [[ $? -ne 0 ]]; then
    echo
    echo "ATENCIÓN: No se detecta la orden 'timeout' en tu sistema. La validación se tendrá que realizar sin hacer uso de esa orden, pero ten en cuenta que en la corrección sí se hará uso de ella."
    echo
    echo "(con las pruebas que se hacen en esta validación, no se permitirá que el programa produzca violaciones de segmento o que tarde demasiado. Si ocurre alguna de estas circunstancias se asumirá que la validación es NEGATIVA. La orden 'timeout' se utiliza para detectar estos casos.)"
    echo
    TIMEOUT=""
fi



# variables to be assigned in each job
num_practica="1"
source="quickSort.cc"
exe="quickSort"
err_file="errors_pr1.log"
warn_file="warnings_pr1.log"
in_tgz_must_be_files="${source} makefile quickSort.gpi"
make_must_build="${exe} quickSort.CPUtimes fit.log quickSort_and_Fit-Function.png quickSort.png"
allowed_files="${in_tgz_must_be_files} ${err_file} ${warn_file}"
objectives_makefile="all quickSort quickSort.CPUtimes graphs"
MIN_TIME=4

#Control variables
#[ ! -z ${TIMEOUT} ] && { TIMEOUT="${TIMEOUT} ${MAX_TIME}"; }
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

[[ $quiet -eq 0 ]] && echo "Entrega de la práctica ${num_practica}"
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
            echo -e "Asegúrate de que la captura mostrada  de DNI/NIE y nombre es correcta."
            echo -e "\nVALIDACIÓN POSITIVA: El trabajo cumple las especificaciones básicas (no se comprueban todas), pero asegúrate de que también compila en los ordenadores del aula."
        else
            echo -e "\nVALIDACIÓN NEGATIVA: Debes corregir los errores antes de entregar."
        fi
#    else
#        echo -n $RETURN_CODE
    fi
    rm -r $tmp_dir
    exit $RETURN_CODE
}


check_err_args() {
    error_message=$($1 2>&1 1>/dev/null)
    ret=$? # 124=timeout; 134=seg-fault; 139=core-dumped
    if [[ ret -eq 124 ]] || [[ ret -eq 134 ]] || [[ ret -eq 139 ]]; then
        echo "Aviso: '$2' produce violación de segmento o 'timeout'" >> ${warn_file}
        warnings=1
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
    check_memory_map --save $1 $2    
}

error_msg(){
    echo -e "\n*** Validación número $3 ***" >> ${err_file}
    echo -e "  Prueba realizada: $2" >> ${err_file}
    echo -e "  $1" >> ${err_file}
    RETURN_CODE=$(( RETURN_CODE |= 1 ))
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


# Verify that has been delivered what is requested
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
[[ -f Makefile ]] && mv Makefile makefile
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
start=$(date +%s)
$TIMEOUT make all &> /dev/null
ret=$?
end=$(date +%s)
elapsed_t=$(echo $(($end - $start)) )
[ $quiet -eq 0 ] && echo -n "Tiempo de proceso (seg.): "; [ $ret -eq 124 ]  && echo "TIMEOUT(${MAX_TIME})" || echo ${elapsed_t}
case $ret in
    124) msg_timeout="'make' se lleva un tiempo excesivo (> ${MAX_TIME} seg.)";;
    134) msg_timeout="El programa produce una violación de segmento";;
    139) msg_timeout="El programa produce una violación de segmento";;
    *)   [[ ret -ne 0 ]] && { msg_timeout="'make' ha fallado ($ret)"; };;
esac
[[ ret -ne 0 ]] && { echo Error: $msg_timeout >> ${err_file}; RETURN_CODE=$((RETURN_CODE |= 1)); exit 1; }

# 'make all' is too fast!
if [[ $elapsed_t -lt $MIN_TIME ]]; then 
    echo "Error: No se ajusta a las especificaciones en cuanto al número de repeticiones, tamaño de las instancias o valores que deben tomar." >> ${err_file} 
    RETURN_CODE=$((RETURN_CODE |= 1))
fi

# verify that 'make' builds all files
check_num=7
are_not=""
for f in ${make_must_build}; do
    if [ ! -f $f ]; then
         are_not=${are_not}" "${f}
    fi 
done
if [[ ! -z ${are_not} ]]; then
    echo "Error: 'make' no crea todo los archivos que se piden (${are_not})." >> ${err_file}
    RETURN_CODE=$((RETURN_CODE |= 1))
fi

# verify objectives in makefile
check_num=8
fault=""
for o in ${objectives_makefile}; do
    if [[ ! $(make -t ${o}) ]]; then
         fault=${fault}" "${o}
    fi 
done
if [[ ! -z ${fault} ]]; then
    echo "Error: Alguno de los objetivos del 'makefile' han fallado (${fault})." >> ${err_file}
    RETURN_CODE=$((RETURN_CODE |= 1))
fi

# verify that only one fit "mmcc" is made
check_num=9
num_fits=$(cat fit.log | grep fitting | wc -l)
[[ $num_fits -ne 1 ]] && { echo "Error: Solo hay que hacer un ajuste de mmcc, el que consideres mejor." >> ${err_file}; RETURN_CODE=$((RETURN_CODE |= 1)); exit 1; }


# verify fit function
check_num=10
funct=$(grep "function used for fitting" fit.log | tail -n 1 | cut -d ':' -f 2)
funct=$(echo ${funct} | cut -d " " -f -1)  
funct=$(grep  ${funct}' '*= fit.log | tail -n 1 | tr -d ' ')
if [[ "$funct" =~ "x*x" ]] || [[ ! "$funct" =~ "log" ]]; then
    echo "Error: La función de ajuste utilizada no es correcta." >> ${err_file}
    RETURN_CODE=$((RETURN_CODE |= 1))
fi


( [[ -f ${err_file} ]] || [[ $((RETURN_CODE&1)) -eq 1 ]] ) && exit 1;

exit 0


