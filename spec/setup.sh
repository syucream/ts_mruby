# pass ATS prefic+'etc/trafficserver' as ATS_ETC_DIR
if [ $ATS_ETC_DIR ]; then
  DEST_DIR=$ATS_ETC_DIR
else
  DEST_DIR=/usr/local/etc/trafficserver/
fi

# generate config files
cd ats_config
find . -type f -print0 | xargs -0 -I% sh -c "ruby % > ${DEST_DIR}/%"
cd ..
