# Iguana
Iguana stands for "Intelligent Guided Image Acquisition and Annotation".


## What is Iguana
Its sole purpose is to automatically download pictures from the internet,
allow a user to attach custom label tags to them, and use fancy deep learning
to predict the labels for any new upcoming downloaded pictures.

The output is the two-fold:
 * A database of pictures downloaded from the internet
 * A deep learning model that can judge any picture, based on the individual user's preferences


## In what state is Iguana at the moment
Right now, the tool consists of a working Qt-based user interface that allows downloading
wallpaper images from 4chan ("/wg/"). Each downloaded image is displayed, and the user can
interactively navigate the downloaded images. The controls are not yet super convenient,
but they do their job.

Each image can be annotated with tags. The tags can be added and removed completely based on
user-preferences.

The deep learning side is not yet implemented - so no prediction of labels yet (but that's in
the works). This will probably be TensorFlow-based.


## Hints
I'm developing this on Windows. This makes a couple of details harder than necessary. One of these
details includes using the C++ TensorFlow libraries (or building them first, for that matter). To
solve that issue, I followed this excellent guide:

 * https://joe-antognini.github.io/machine-learning/build-windows-tf
   This will include installing some more components, including Anaconda and CMake.

And let myself be inspired by Joe's tutorial on the very same library afterwards:

 * https://joe-antognini.github.io/machine-learning/windows-tf-project

Qt can of course be downloaded from their official website:

 * https://www.qt.io/download

How you compile this is up to you, but the Qt Creator worked fine for me using the Open Source
variant of the SDK and tools.

For TensorFlow in its current state, you need Python 3.5. Anaconda comes with Python 3.6, for which TensorFlow is
not yet supported. You can downgrade Anaconda's Python pretty easily - their FAQ tells you how:

 * https://docs.anaconda.com/anaconda/faq#how-do-i-get-anaconda-with-python-3-5

It takes a while to download/install, but its dead simple.
