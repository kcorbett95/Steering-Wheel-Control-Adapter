This project is meant to adapt steering wheel media control buttons to aftermarket stereo head-units. It requires the head-unit to use an analog remote
input, usually in the form of a 3.5mm headphone jack in the back of the unit, and the vehicle steering wheel buttons to be a resistor-network type where
each button corresponds to a different resistance value between two wires. Vehicles with many buttons often have 2 input wires and a 3rd reference wire,
where the resistances can be between the reference wire and either input wire, doubling the number of input buttons for the same resistances.

The existing code is currently working on a 2015 Subaru Crosstrek/Impreza and a Pioneer AVH4500NEX head unit. You will need to consult the vehicle
factory service manual to determine what the resistance values for each steering wheel button is, or do some testing with a multimeter to measure each
button manually (this takes forever, don't do this unless you have no other option.) You will also need to know the remote input values the head unit
needs for each function. This information is kindof difficult to find, but I found the values for Pioneer head units after a few hours of digging through
forums and such. An email to the support of your head-unit manufacturer might be helpful in finding this information.

************   MAPPING FUNCITONS    **************
Once you have the required resistance values for both your vehicle, and the corresponding head unit values, the mapping procedure is fairly easy:

1) In the beginning of the SWC_adapter.ino arduino file, you will find the definitions for the head unit values. You will need to replace these values
with the ones for your head unit. 

For example, if your volume up function needs a 55kOhm input, replace the value: 

#define volUp 55000

Repeat this process for the rest of the defined functions. If your vehicle does not have a button corresponding to one of these values, you can either
replace the value with "noBtn" without the quotes if you do not wish to use this function, or you can make a custom value for a button-hold to implement
a different function in later steps (more advanced).

