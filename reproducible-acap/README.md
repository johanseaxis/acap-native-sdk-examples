 *Copyright (C) 2021, Axis Communications AB, Lund, Sweden. All Rights Reserved.*

# An example of how to create a reproducible ACAP application
This README file explains how to build a simple and reproducible ACAP application. It is achieved by using the containerized Axis API and toolchain images.

Together with this README file, you should be able to find a directory called app. That directory contains the "reproducible-acap" application source code which can easily be compiled and run with the help of the tools and step by step below.

The example follows the guidelines in [Reproducible builds practices](https://reproducible-builds.org/) but might not be enough for more complex use cases.

## Getting started
These instructions will guide you on how to execute the code. Below is the structure and scripts used in the example:

```bash
reproducible-acap
├── app
│   ├── reproducible_acap.c
│   ├── LICENSE
│   ├── Makefile
│   └── manifest.json
├── Dockerfile
└── README.md
```

* **app/reproducible_acap.c** - Hello World application which writes to system-log.
* **app/LICENSE** - Text file which lists all open source licensed source code distributed with the application.
* **app/Makefile** - Makefile containing the build and link instructions for building the ACAP application.
* **app/manifest.json** - Defines the application and its configuration.
* **Dockerfile** - Docker file with the specified Axis toolchain and API container to build the example specified.
* **README.md** - Step by step instructions on how to run the example.

### Build the application
The following build steps are to show the reproducibility and not how a real
application would be built.

Standing in your working directory run the following commands:

> [!IMPORTANT]
> *Depending on the network you are connected to,
The file that needs those settings is: *~/.docker/config.json.*
For reference please see: https://docs.docker.com/network/proxy/ and a
[script for Axis device here](../FAQs.md#HowcanIset-upnetworkproxysettingsontheAxisdevice?).*


```bash
docker build --build-arg TIMESTAMP="$(git log -1 --pretty=%ct)" --tag rep1 .
docker cp $(docker create rep1):/opt/app ./build1
```
We started with an ordinary build which will not give a reproducible package and
copied the result from the container image to a local directory *build1*.

Now let's create a reproducible package and add option `--no-cache` to ensure
that it's rebuilt. We set the build argument TIMESTAMP which in its turn set
[SOURCE_DATE_EPOCH](https://reproducible-builds.org/docs/source-date-epoch/)
to a fix time. The chosen timestamp here is the latest commit in the current
git repository. We copy the output to *build2*.
```bash
docker build --build-arg TIMESTAMP="$(git log -1 --pretty=%ct)" --tag rep2 .
docker cp $(docker create rep2):/opt/app ./build2
```

Build a second reproducible application and copy the output to *build3*.
```bash
docker build --build-arg TIMESTAMP="$(git log -1 --pretty=%ct)" --tag rep3 .
docker cp $(docker create rep3):/opt/app ./build3
```

Now you will have three eap-files in your working directory
```bash
reproducible-acap
├── build1
│   └── reproducible_acap_1_0_0_armv7hf.eap
├── build2
│   └── reproducible_acap_1_0_0_armv7hf.eap
├── build3
│   └── reproducible_acap_1_0_0_armv7hf.eap
```
* **build1/reproducible_acap_1_0_0_armv7hf.eap** - Application package .eap file.
* **build2/reproducible_acap_1_0_0_armv7hf.eap** - Reproducible application package .eap file.
* **build3/reproducible_acap_1_0_0_armv7hf.eap** - Reproducible application package .eap file.

Compare the files to see if they are identical or not.
```bash
cmp build1/*.eap build2/*.eap
build1/reproducible_acap_1_0_0_armv7hf.eap build2/reproducible_acap_1_0_0_armv7hf.eap differ: byte 13, line 1

cmp build2/*.eap build3/*.eap
```

#### Build the application interactively
To get reproducible packages running inside a container, make sure to export the
environment variable before running build command.
```bash
SOURCE_DATE_EPOCH=$(git log -1 --pretty=%ct) acap-build .
```
**N.b.** To be able to use the git log as in this example you will have to run
the docker container from the top directory where the `.git` directory is placed.  


## License
**[Apache License 2.0](../LICENSE)**
