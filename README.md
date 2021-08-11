# Hypatia: A meteorological data logger capable of collecting and storing data and metadata.

## Introduction

Typically, data loggers (meteorological and not) collect "raw" sensor readings and process them to form useful measurements. This processing, very often an averaging in the time domain or the calculation of some other statistic, is almost always "destructive": once you get the result, you cannot deduce from it the value of the raw readings it was computed on.

And this is a source of potential troubles: although not commonly used in practical applications, many metadata are lost. Among them, indications about noise in readings, or glitches and other conditions affecting the results' quality.

The Hypatia logger is an attempt to overcome this problem: in addition to averages and other common statistics, it also collects information about aspects of the raw data distribution relevant to data quality, and transform them to maintenance and validation oriented metadata which users may employ on their will.

## Why the name, _Hypatia_

Hypatia (c.355-415 AD) was a renown mathematician, astronomer and philosopher active in Alexandria, then one of the cultural centres of the classical world.

Many books and papers have been recently written on her figure, underlining the sheer power of her mind, the with-facts opposition to the rampant sexism of these days, and the tragic circumstances of her death. You will have ho difficulty locating her biographies on the web (see for example https://www.britannica.com/biography/Hypatia).

But besides being the first female mathematician of whom history has things to say, the reason of naming this repository after her stems from one of her leading ideas: knowledge can, and should be, disseminated to anyone willing to learn and use it.

In her times, this contrasted with the assumptions that knowledge would have had better to be restrained to few priests and philosophers, as it can be dangerous to untrained minds.

Today, this assumption is still in charge, although few people would profess it explicitly. This is especially true in technology, with the massive diffusion of embedded systems (data loggers among them) whose design is based on the principle of "information hiding": according to it, devices gets easier to use and understand if their irrelevant innards are not exposed.

But, who decides which information is irrelevant?

Most typically the designers themselves. Sometimes the companies building the systems.

Rarely designers, who most often have engineering backgrounds, know exactly the concerns and needs of users, especially in the research community. And this yields the very concrete dangers that some of the information hidden was not that "irrelevant".

Curiously, users themselves can be largely unaware of the problem: not being engineers, they may just imagine no information hiding has occurred during the design, so that what they do not see simply does not exist.

Maybe, in order to fill the hyatus between the worlds of research and engineering a new Hypatia would be highly needed to date. But waiting for her, in the meanwhile we could modestly try to do some little practically-minded steps, boiling down to the necessity to a better communication.

## Knowledge is in the raw data

Let's consider a standard meteorological station, equipped with an anemometer, a thermo-hygrometer, a rain gauge and, maybe, a pyranometer.

As the mainstream technology would suggest, all meteorological quantities read by the station sensor vary "slowly", so it makes sense to collect data at a relatively quiet rate (say once per 30 seconds), then compute averages on some appropriate interval, maybe 10 minutes, or 1 hour.

In reality, all meteorological quantities (you may think wind and temperature as intuitive examples) are subject to turbulent fluctuations, so that large natural variations in data could be appreciated even at time scales in the order of one tenth of a second or less. The fact this variation is not seen at the output clamp of mainstream sensors comes from a (desired or not) design decision: mainstream sensors act as low-pass filters. The filter cut frequency stems from moments of inertia of the sensor parts, frictions, thermal inertias, and the like. Sensor designers can control them, to some degree: practical constraints need also to be taken into account, first and foremost that the sensor is robust enough to survive the usually harsh meteorological environments on this planet.

In addition to the intrinsic turbulent variation of meteorological quantities, it may (and does) happen that changes in measured vaues occur because of electrical phenomena possibly occurring at any level of the measurement chain. We may conceptualize this part of variation not being natural, but the consequence of some failure, either temporary of definitive. Although not "natural", this variation is not unexpected: after all, any sensor can be thought as a sort of natural rock, and geologists tell us all rock weather, sooner or later. Wait anough, and any manufact will fail, as its components deteriorate by rock weathering.
