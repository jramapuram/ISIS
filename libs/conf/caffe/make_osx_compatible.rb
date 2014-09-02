#!/usr/bin/ruby

FILE_PATH = "../../caffe/src/caffe/net.cpp"
COMMENT_OUT_TESTS = "#include \"caffe/test/test_caffe_main.hpp\""

text = File.read(FILE_PATH)
replace = text.gsub(COMMENT_OUT_TESTS, "//#{COMMENT_OUT_TESTS}")
File.open(FILE_PATH, "w") {|file| file.puts replace}

imagenet_dir    = "../../caffe/examples/imagenet"
imagenet_model  = imagenet_dir + "/caffe_reference_imagenet_model"
imagenet_script = "./get_caffe_reference_imagenet_model.sh"
unless File.exist?(imagenet_model)
	Dir.chdir(imagenet_dir){
		%x[#{imagenet_script}]
	}
end

ilsvrc12_dir    = "../../caffe/data/ilsvrc12"
imagenet_synset = ilsvrc12_dir + "/synset_words.txt"
synset_script   = "./get_ilsvrc_aux.sh"
unless File.exist?(imagenet_synset)
	Dir.chdir(ilsvrc12_dir){
		%x[#{synset_script}]
	}
end
