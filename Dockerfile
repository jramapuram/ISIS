FROM ubuntu:14.04

MAINTAINER Jason Ramapuram <jason.ramapuram@gmail.com>

ENV DEBIAN_FRONTEND noninteractive

ENV CUDA_DRIVER 343.36
ENV CUDA_VER 6.5.14

ENV CUDA_INSTALL http://us.download.nvidia.com/XFree86/Linux-x86_64/${CUDA_DRIVER}/NVIDIA-Linux-x86_64-${CUDA_DRIVER}.run
ENV CUDA_RUN http://developer.download.nvidia.com/compute/cuda/6_5/rel/installers/cuda_${CUDA_VER}_linux_64.run

# Update and install minimal.
RUN apt-get update -qqy
RUN apt-get install -qqy \
       build-essential \
       module-init-tools \
       wget \ 
       unzip \
       cmake \
       ca-certificates \
       openssh-client \
       libopencv-dev \
       qt5-default \
       libboost-all-dev \
       g++-4.6 \
       bc \
       ruby \
       libopenblas-base \
       libopenblas-dev \
       emacs24-nox \
       dh-autoreconf \
       libleveldb-dev \
       liblmdb-dev \
       libsnappy-dev \
       libhdf5-serial-dev \
       libprotobuf-dev \
       protobuf-compiler \
       curl \
       python-pip \
       git

# Clean up packages.
#RUN apt-get clean && rm -rf /var/lib/apt/lists/*

# Install nvidia Drivers & CUDA.
RUN wget $CUDA_INSTALL -P /tmp --no-verbose && chmod +x /tmp/NVIDIA-Linux-x86_64-${CUDA_DRIVER}.run
RUN wget $CUDA_RUN -P /tmp --no-verbose && chmod +x /tmp/cuda_${CUDA_VER}_linux_64.run
RUN /tmp/NVIDIA-Linux-x86_64-${CUDA_DRIVER}.run -s -N --no-kernel-module
RUN /tmp/cuda_${CUDA_VER}_linux_64.run -silent -samples -toolkit
RUN rm -rf /tmp/*

WORKDIR /opt

# Glog 
RUN wget --no-check-certificate https://google-glog.googlecode.com/files/glog-0.3.3.tar.gz && \
  tar zxvf glog-0.3.3.tar.gz && \
  cd /opt/glog-0.3.3 && \
  ./configure && \
  make && \
  make install

# Protobuf
#RUN git clone https://github.com/google/protobuf
#RUN cd protobuf && ./autogen.sh && ./configure && make && make install

# Workaround for error loading libglog: 
#   error while loading shared libraries: libglog.so.0: cannot open shared object file
# The system already has /usr/local/lib listed in /etc/ld.so.conf.d/libc.conf, so
# running `ldconfig` fixes the problem (which is simpler than using $LD_LIBRARY_PATH)
# TODO: looks like this needs to be run _every_ time a new docker instance is run,
#       so maybe LD_LIBRARY_PATh is a better approach (or add call to ldconfig in ~/.bashrc)
RUN ldconfig

# Gflags
RUN cd /opt && \
  wget --no-check-certificate https://github.com/schuhschuh/gflags/archive/master.zip && \
  unzip master.zip && \
  cd /opt/gflags-master && \
  mkdir build && \
  cd /opt/gflags-master/build && \
  export CXXFLAGS="-fPIC" && \
  cmake .. && \ 
  make VERBOSE=1 && \
  make && \
  make install

# Python related
pip install pyyaml urllib3 hashlib argparse

# Build ISIS
RUN mkdir ISIS
ADD . /opt/ISIS/
RUN cd ISIS && mkdir build && cd build && cmake .. && make clean && make
#RUN git clone https://github.com/BVLC/caffe.git
#RUN cd caffe && cp Makefile.config.example Makefile.config
#RUN make all

# Numpy include path hack - github.com/BVLC/caffe/wiki/Setting-up-Caffe-on-Ubuntu-14.04
#RUN NUMPY_EGG=`ls /usr/local/lib/python2.7/dist-packages | grep -i numpy` && \
#  ln -s /usr/local/lib/python2.7/dist-packages/$NUMPY_EGG/numpy/core/include/numpy /usr/include/python2.7/numpy

# Build Caffe python bindings and make + run tests
#RUN cd caffe && make pycaffe

# Default command.
ENTRYPOINT ["/bin/bash"]
