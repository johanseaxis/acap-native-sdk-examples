 *Copyright (C) 2021, Axis Communications AB, Lund, Sweden. All Rights Reserved.*

# ACAP3 applications interacting with external library
This README file explains how to use the external library, which provides an interface to the system found in Axis device.

The purpose is to provide applications a mechanism to use external library.

One examples have been added to illustrate some use cases of the external library.

The example will create an application that fetches the content from URL and store it in user defined path.

APIs specification is available on https://curl.se/libcurl/c

## Getting started
Below is the structure of the files and folders on the top level:

```bash
utility_libraries
├── curl_example
└── README.md
```

* **curl_example** - Folder containing files for building ACAP3 application "curl_example".
* **README.md** - Step by step instructions on how to use the examples.

### Example applications
Each example has as a README file in its directory which shows overview, example directory structure and step-by-step instructions on how to run applications on the device.
Below is the list of examples available in the repository.

* [Curl Example](./curl_example/README.md)
  * The example code is written in C which fetches file content from URL.

## License
**[Apache License 2.0](../LICENSE)**
