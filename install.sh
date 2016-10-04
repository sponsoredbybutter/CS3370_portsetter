#!/bin/bash

var_directory=$PWD

echo "Adding setport as a command!"

cat > setport << END_FILE
#!/bin/bash

${var_directory}/portsetter.cpp.o \$@
END_FILE

chmod +x setport
sudo mv setport /usr/bin/

echo "Done."