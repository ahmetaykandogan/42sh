#!/bin/sh

#$1 file
#$2 test description
#$3 test

echo "#!/bin/sh" > $1
echo >> $1
echo -n "# " >> $1
echo $2 >> $1
echo >> $1
echo $3 >> $1

chmod 777 $1
