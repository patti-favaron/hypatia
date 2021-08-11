# Hypatia: A meteorological data logger capable of collecting and storing data and metadata.

Typically, data loggers (meteorological and not) collect "raw" sensor readings and process them to form useful measurements. This processing, very often an averaging in the time domain or the calculation of some other statistic, is almost always "destructive": once you get the result, you cannot deduce from it the value of the raw readings it was computed on.

And this is a source of potential troubles: although not commonly used in practical applications, many metadata are lost. Among them, indications about noise in readings, or glitches and other conditions affecting the quality of the results.
