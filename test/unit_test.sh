
gcc -c unit_test_main.c -I ../inlcude/*.h 
if [ "$?" == "0" ]; then
    gcc -c ../enjoy/feature.c -I ../include/*.h 
else
    echo "error"
    exit 1
fi
gcc -c ../enjoy/error.c -I ../include/*.h
gcc -c ../enjoy/flow.c -I ../include/*.h
if [ "$?" == "0" ]; then
    gcc unit_test_main.o feature.o error.o flow.o -o unit_test
else
    echo "error"
    exit 1
fi
