dvb:
	$(MAKE) -C gst-dvb
	$(MAKE) -C gst-dvb inspect

csa:
	$(MAKE) -C csasys

osc:
	$(MAKE) -C client-oscam

demuxcsa: csa
	$(MAKE) clean -C gst-demux
	$(MAKE) -C gst-demux DECRYPT=CSASYS
	$(MAKE) -C gst-demux inspect

demuxoscam: osc
	$(MAKE) clean -C gst-demux
	$(MAKE) -C gst-demux DECRYPT=OSCAM
	$(MAKE) -C gst-demux inspect

demux-patch:
	$(MAKE) -C gst-demux ver-patch

dvb-install:
	$(MAKE) -C gst-dvb install

demux-install:
	$(MAKE) -C gst-demux install

clean:
	$(MAKE) clean -C gst-dvb
	$(MAKE) clean -C csasys
	$(MAKE) clean -C client-oscam
	$(MAKE) clean -C gst-demux
