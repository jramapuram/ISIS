#!/usr/bin/ruby

FILE_PATH  = "../../caffe/Makefile"
ERROR_LINE = "CXX ?= /usr/bin/g++"

text = File.read(FILE_PATH)
replace = text.gsub(ERROR_LINE, "CXX := /usr/bin/g++-4.6")
File.open(FILE_PATH, "w") {|file| file.puts replace}

imagenet_dir    = "../../caffe/models/bvlc_reference_caffenet"
imagenet_model  = imagenet_dir + "/bvlc_reference_caffenet.caffemodel"
imagenet_script = "./get_caffe_reference_imagenet_model.sh #{imagenet_dir}"
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
