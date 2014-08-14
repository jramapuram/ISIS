/*
 * autoencoder.h
 *
 *  Created on: Sep 4, 2013
 *      Author: jason
 */

#ifndef AUTOENCODER_H_
#define AUTOENCODER_H_

#include <iostream>
#include <string>
#include <Python.h>
#include <boost/python.hpp>

using namespace boost::python;

class autoencoder {
private:
	const int DefaultInputUnits = 80*60; //scaled version of the image [tested to prove no OOM]
	const int DefaultHiddenUnits = (DefaultInputUnits) / 2; //default image size, hidden is half this size
	const std::string DefaultFileName="AI.npy";

	int HiddenUnits, InputUnits;
	std::string FileName;
	object autoEnc; //actual python autoencoder object

	void construct() {
		try {
			Py_Initialize();
			object main_module = import("__main__");
			object main_namespace = main_module.attr("__dict__");

			//Train a DeepComposedAutoencoder that consists of 2 HigherOrderContractiveAutoencoder :)
			boost::python::str cmd (
					"from pylearn2.models.autoencoder import HigherOrderContractiveAutoencoder\n"
					"from pylearn2.models.autoencoder import DeepComposedAutoencoder\n"
					"from pylearn2.corruption import GaussianCorruptor\n"
					"from pylearn2.costs.autoencoder import MeanSquaredReconstructionError\n"
					"from pylearn2.training_algorithms.sgd import SGD\n"
					"from pylearn2.termination_criteria import EpochCounter\n"
					"from pylearn2.datasets.npy_npz import NpyDataset\n"
					"from pylearn2.scripts.train import Train\n"

					"models = [HigherOrderContractiveAutoencoder("
					"corruptor=GaussianCorruptor(stdev=.1),"
					"num_corruptions=1,"
					"nvis= "+ std::to_string(InputUnits) +
					",nhid= "+ std::to_string(HiddenUnits)+
					", act_enc= 'sigmoid',act_dec= 'sigmoid'),"

					"HigherOrderContractiveAutoencoder("
					"corruptor=GaussianCorruptor(stdev=.1),"
					"num_corruptions=1,"
					"nvis= "+ std::to_string((HiddenUnits)) +
					",nhid= "+ std::to_string(HiddenUnits/2)+
					", act_enc= 'sigmoid',act_dec= None)]\n"

					"AImodel=DeepComposedAutoencoder(models)\n"

					"AIModelData=NpyDataset(file='"+FileName+"')\n"

					"trainAlg=SGD(learning_rate=1e-3,batch_size=10,monitoring_batches=5,monitoring_dataset=AIModelData,cost=MeanSquaredReconstructionError(),termination_criterion=EpochCounter(max_epochs=50))\n"

					"trainObj=Train(dataset=AIModelData,model=AImodel,algorithm=trainAlg,save_path='./AI.pkl',save_freq=10)"
			);
			autoEnc = exec(cmd, main_namespace);

		} catch (error_already_set& e) {
			PyErr_PrintEx(0);
		}
	}

public:
	/**
	 * Default constructor to init with 'DefaultLayers'
	 */
	autoencoder() {
		this->HiddenUnits = DefaultHiddenUnits;
		this->InputUnits = DefaultInputUnits;
		this->FileName=DefaultFileName;
		construct();
	}

	/**
	 * Initialize with custom Layers [TBD]
	 */
	autoencoder(int inputLayers, int hiddenLayers, std::string dataFileName) {
		this->HiddenUnits = hiddenLayers;
		this->InputUnits = inputLayers;
		this->FileName=dataFileName;
		construct();
	}

};

#endif /* AUTOENCODER_H_ */
