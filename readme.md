# Barcode39 Generator

This library generates Code 39 barcodes in C. The project is in beta version.


char *originalString = "DAVIDE";

// Create options.
barcodex39opt opt;

opt.width  = 400;

opt.height = 100;

opt.mode = Fixed;

opt.showText = false;

opt.narrowLineWidth = 1;

//Generate the bar code.
barcode39Data* barcodeData = generate(opt, originalString);

//Save the barcode
savepng(barcodeData,"barcode.png");


## Compilazione

```sh
make
