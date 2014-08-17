#!/usr/bin/ruby

FILE_PATH = "../../caffe/src/caffe/net.cpp"
COMMENT_OUT_TESTS = "#include \"caffe/test/test_caffe_main.hpp\""

text = File.read(FILE_PATH)
replace = text.gsub(COMMENT_OUT_TESTS, "//#{COMMENT_OUT_TESTS}")
File.open(FILE_PATH, "w") {|file| file.puts replace}