#!/bin/sh
CWD="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
O_FILE=$CWD/loc_map.txt
PRE_PARSED_FILE=$CWD/tmp.txt
topology --io | sed -r '/^\s*$/d' | tail -n +3 | awk '{print $4","$2}' > $PRE_PARSED_FILE

if [ -f "$O_FILE" ]
then
    rm "$O_FILE"
fi

# we need to asssociate the missing location with the one above it


# $1 = str, $2 = deliminator
function split_str(){
    IN=$1
    ADDR=(${IN//,/ })
}

pci_addr=''
loc=''
last_loc=''
ADDR=

# main()
while IFS= read -r line
do
    split_str $line ,
    pci_addr=${ADDR[0]}
    loc=${ADDR[1]}

    # first iteration
    if [ -z $last_loc ]
    then
        last_loc="$loc"
    fi

    # what if loc= '.'
    if [ $loc = '.' ]
    then
        loc=$last_loc
    else
        last_loc=$loc
    fi
    echo "$pci_addr,$loc" >> $O_FILE
done < $PRE_PARSED_FILE
