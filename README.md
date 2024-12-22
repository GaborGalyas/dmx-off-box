DMX OFf Box

A quick Black Box I just put together.
It is basically a dmx disconnector switch. 
The dmx simply passes trough, and when it gets the correct value, a relay disconnects the output side. 
My main purpose for this is to have it in line with our haze machines, and after show I send an “off” so it disconnects dmx signal, and hazers enter their purge and clean mode. 
By the time I get on stage to pack up, they are ready to be put away. 
It supports RDM for addressing, and selecting modes which are 1ch Normally Open, 1ch Normally Closed and 2ch Set/Reset mode.

Software is a bit of a remake of my other product, so the code is messy and has other experimental stuff too. Feel free to clean up.
I included the DmxSerial2 library as it has some changes for my other product that uses DIP switches for addressing aswell.
The switching point can be set with modifying the parameter LAMP HOURS with an rdm manager tool, or by changing the values in the code.