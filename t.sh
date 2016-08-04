# for rspec. require ATS_PREFIX as ATS prefix + 'etc/trafficserver'

set -eu

if [ $ATS_PREFIX ]; then
  PREFIX=$ATS_PREFIX
else
  PREFIX=/usr/local/
fi
ETC_DIR=${ATS_PREFIX}etc/trafficserver/

## setup gems
gem install bundler
bundle install

## execute rspec
cd spec/
ATS_ETC_DIR=$ETC_DIR sh ./setup.sh
${PREFIX}bin/traffic_server &
bundle exec rspec features
cd ../
