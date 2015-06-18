# script to send multiple client requests in parallel
./qaclient "what is the speed of light?" 9081 &
./qaclient "who is the President of the United States?" 9081 &
#./qaclient "what is the speed of sound?" &
