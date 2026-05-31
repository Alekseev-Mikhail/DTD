#Generate mocks
ruby auto/cmock.rb -o"config/CMockConfig.yml" ../src/utility/istdlib.h

#Generate test runners
ruby auto/generate_test_runner.rb test/test_list.c gen/test_list.c
