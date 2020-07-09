#!/bin/sh

PRE_PARSED_FILE='tmp.txt'
topology --io | sed -r '/^\s*$/d' | tail -n +3 | awk '{print $4","$2}' > $PRE_PARSED_FILE
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
    echo "$pci_addr,$loc" >> loc_map.txt
done < $PRE_PARSED_FILE
