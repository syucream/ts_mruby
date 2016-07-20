# pass ATS prefic+'etc/trafficserver' as ATS_ETC_DIR
if [ $ATS_ETC_DIR ]; then
  DEST_DIR=$ATS_ETC_DIR
else
  DEST_DIR=/usr/local/etc/trafficserver/
fi

# generate and copy ATS config files
find files -name '*.config.rb' -print0 | perl -pe 's/\.rb\0/\0/g' | xargs -0 -I% sh -c 'ruby %.rb > %'
mv -f files/*.config ${DEST_DIR}

# generate and copy mruby scripts
mkdir -p files/scripts/
find files/templates/ -name '*.rb' -print0 | perl -pe 's/.+\///g' | xargs -0 -I% sh -c 'ruby files/templates/% > files/scripts/%'
cp -r files/scripts ${DEST_DIR}
