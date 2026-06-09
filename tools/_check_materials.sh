find $1 -name "*.model" -exec echo {} \; -exec sh -c "./edward file check {} | grep XYZ" \;

