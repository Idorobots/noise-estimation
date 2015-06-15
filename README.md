# Noise Estimation
Noise estimation in fMRI signals. An AGH-UST project.

# Building & running
To build the project simply run the following command in your terminal:

```
$ make clean && make
```

The supplied makefile will compile the project. Required tools & libraries:
- **GNU Make** (tested using GNU Make 4.1),
- **GCC** (tested using gcc (GCC) *5.1.0*),
- **OpenCV** (tested with version *2.4.10*)
- POSIX-compatible OS (tested using Arch w/ *Linux 4.0.5-1-ARCH*).

To run the project use *run* target of the Makefile:
```
$ make run
$ make run CONFIG=path/to/config.conf
```
...or run the executable directly:

```
$ ./ne path/to/config.conf
```

# Available CLI & configuration options:

To list the available command line options run:
```
$ ./ne --help
```

Output:
```
USAGE: ./ne [OPTIONS] CONFIG_FILE
OPTIONS:
	--help         Display this message.
	--no-gui       Disable GUI images.
	--time N       Disable GUI images & time N executions.
```

Available configuration options (usabe via configuration file):

- **smooth_window_size** - optional, selects window size for local mean filter,

- **ex_filter_type** - required, selects filtering type (1 = local mean, 2 = expectation maximization),
- **ex_window_size** - required, selects window size for filters in expectation maximization,
- **ex_iterations** - required, selects number of iterations for expectation maximization,

- **lpf_f** - required, specifies the sigma of the low-pass gaussian filter used to preprocess the input image,
- **lpf_f_SNR** - required, specifies the sigma of the low-pass gaussian filter used to process the estimated SNR map,
- **lpf_f_Rice** - required, specifies the sigma of the low-pass gaussian filter used in rician correction,

- **input_filename** - required, specifies the name of the noisy input image (can be either CSV or PNG),
- **input_filename_SNR** - optional, specifies the name of the SNR map (can be either CSV or PNG), if the file specified by this option does not exist, the application will attempt to estimate the SNR map,
- **output_filename_Gaussian** - required, specifies the name of the output filename that will store the gaussian noise map estimation (can be either CSV or PNG),
- **output_filename_Rician** - required, specifies the name of the output filename that will store the rician noise map estimation (can be either CSV or PNG),

- **csv_delimiter** - optional, specifies the delimiter of the CSV files processed by the application,

- **title_input** - optional, specifies the title for the GUI window containing the noisy input image,
- **title_SNR** - optional, specifies the title for the GUI window containing the SNR map,
- **title_Gaussian** - optional, specifies the title for the GUI window containing the gaussian noise estimation map,
- **title_Rician** - optional, specifies the title for the GUI window containing the rician noise estimation map,

# Algorithm timings
The following timings were captured on a machine with the following specifications:
```
OS:     Arch Linux
Kernel: x86_64 Linux 4.0.5-1-ARCH
Shell:  bash 4.3.39
RAM:    3734MB
CPU:    Intel Pentium CPU B950 @ 2.1GHz
```

Each test has been run with the following command (after appropriately adjusting the configuration file):
```
$ ./ne --time 50 config.conf
```

## Timings for local mean & known SNR:
```
Running 50 tests:
  Total time:   3159.140000 ms
  Average time: 63.182800 ms
  Maximal time: 66.875000 ms
  Minimal time: 62.418000 ms
```

## Timings for local mean & estimated SNR:
```
Running 50 tests:
  Total time:   19067.734000 ms
  Average time: 381.354680 ms
  Maximal time: 387.622000 ms
  Minimal time: 379.153000 ms
```

## Timings for expectation maximization & known SNR:
```
Running 50 tests:
  Total time:   18359.728000 ms
  Average time: 367.194560 ms
  Maximal time: 375.301000 ms
  Minimal time: 365.145000 ms
```

## Timings for expectation maximizations & estimated SNR:
```
Running 50 tests:
  Total time:   18943.468000 ms
  Average time: 378.869360 ms
  Maximal time: 386.274000 ms
  Minimal time: 376.519000 ms
```

# Known issues

- Modified Bessel function implementation used in this project is fairly inaccurate and leads to larger errors in the expectation maximization case.
