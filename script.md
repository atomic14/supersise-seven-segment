The boards arrived from PCBWay a week or so ago and they look great. I've been slowly assembling them in the evenings.

This should be an interesting video - so don't touch that dial!

First I'll quickly run through the assembly -

We've got the digits - both large low voltage and smaller high voltage versions.

And we've got the high voltage boost circuit circuit - this takes our 5v and boosts it up to around 46 volts to drive the small digits

I'll quickly run through the code.

We're using an NTP server to get the current date and time and I've added a very simple stopwatch just for fun. 

With the code in place the clock works really nicely so we'll see it in all its glory.

And then of course I've got the inevitable bunch of mistakes that I've made in both the design and execution of the project.

I've covered the design of the boards in a previous set of videos - links are in the description.

Assembly

Let's get on with the assembly.

I knew the large digits would be quite big, but I wasn't quite prepared for how large they are.

Soldering them up is pretty easy, there is a front and a back to these LED filaments - it doesn't really make much difference when running them at full brightness, you only really see a difference when running at low current, but for a completely consistent display I've tried to get them all the same way round.

I've taken the same approach as I would with SMD assembly.

I'm adding some solder to one of the pads and then melting the solder while I slide the connector onto the pad.

With one of the ends attached we just solder the other end without any problems.

Adding a bit of flux to the LED filament connectors really helps with this. I've also cranked up the soldering iron temperatrue a bit as there's quite a lot of metal to get warm.

The positive end of the filament is denoted by these small holes. Inevitably I managed to mess a couple of these up, but it's quite easy to desolder and redo them.

All the filements work - it's fairly easy to test this without any code by just connecting a wire to ground and then touching each resistor - I was slightly worried that there would be some factory rejects mixed in with my batch but they all seem fine.

Somewhat couterintitively the larger filaments run off 3v and the smaller filaments need around 46 volts to light up. I've run through the boost circuit in another video and much to my surprise it worked pretty much first time.

One important thing that I forot when I first powered up the circuit is this pull down resistor on the gate of the MOSFET - I almost cooked the MOSFET as it was turning itself on and shorting the power supply to ground. Always remember to include a pulldown resistor on the gate of your MOSFETs.

I've got the oscilloscope wired up to the PWM signal coming from the ESP32 and as turn the feedback resistor up and down our voltage goes up and down nicely.

I've turned all the LEDs on and I can slowly ramp up the voltage until they light up nicely.

Running the LEDs through a test pattern the voltage stays nice and consistent even as we turn LEDs on and off. It works just as designed.

Unplugging the high voltage digits and replacing them with the low voltage large digits works just as well.

For the software I'm using the built in library to get the time from an NTP server. Network Time Protocol servers are publicly accessible machines that you can use to get the current time. Fortunately we don't need to worry about how this works as the library takes care of all the details for us - but if you're interested I'll do a video on the subject.

We connect to the WiFi network and then configure our time source with the ntp server along with offsets for our timezone and daylight savings time.

We can then get the current time and pull out the hours and minutes and the date.

I've added a couple of modes to the clock - we can show the current time with hours and minutes, or we can show the month and day, and I've added a little stopwatch. I've set this up to show seconds and hundredths of a second. You can start, stop and clear it. And when you're done you can switch back to the time.

This UI is controlled by a very simple state machine. These are a great way to manage a user interface - I might do a video in the future on these as they are a very powerful concept.

So the project works - but there's definitely room for improvement.

I covered a bunch of these improvements in a previous video, but now I have the boards completed there are more things I'd like to do.

With all the 3v LEDs running we need to supply about 4amps in total. Thi is too much current to really pull through the USB connector on the ESP32 board I'm using. The diode that prevents us feeding voltage back into the USB supply is only rated for 600mA and my computer also gets pretty upset when you try and pull too much current through the USB port.

I had thought about this in advance and added a barrel jack so I could connect a beefy 5v supply to the board.

Unfortunately, if you forget to connect this the board tries to pull all it's power through the USB supply.

The addition of a cheap diode to the circuit would have prevented this problem. I'm quite tempted to do a bit of a hack and cut the track and solder one on.

Another slightly annoying thing I've done is that I added these nice test pads to aid any debugging. They have proved very handy, but you have to hold the probes in place to actually measure anything, it would have been much better to do these as either pins or loops of wire that could be hooked on to.

I'm also not too happy with these two indicator LEDs - the 5v indicator is fine - it tells us if there board is receiving power or not.

The high voltage indicator is less useful - even when we don't have the high voltage being generated there's still 5 volts coming through the inductor. So the LED lights up anyway making it pretty useless.

To be honest, I'm not sure how to indicate if the high voltage supply is working - any suggestions in the comments please.

The last thing that has occurred to me is that I should have made the PCB a lot more modular. Each digit could be its own PCB with the shift register and resistors on it. This would have made the board considerably cheaper to have manufactured as it would have been a lot smaller. It would also have given me a lot more flexibility to buid a nice case and I could have easily added more digits.

I'm starting to think there's a follow up video with version two coming soon. Let me know if this would be interesting.

Apart from these minor niggles the project has come out really nicely. The schematic and board layouts are all on GitHub along with the software.

Thanks for watching and I'll see you in the next video.



