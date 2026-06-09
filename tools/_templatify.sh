find $1 -name "*.model" -exec echo {} \; -exec sh -c "./edward templatify {} > /dev/null" \;

