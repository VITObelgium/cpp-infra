# create a conda package like this

# create a build environment to perform the build (do this once)
conda create --name build
conda install conda-build conda-verify anaconda-client

# activate the environment
conda activate build
# in recipe dir
conda-build .