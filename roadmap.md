## Enhancements:
- [x] Add a README.md.
- [ ] Make a general README with a desctiption of the whole system (but instructions for only this one).
- [ ] Come up with a good name.
- [x] Debloat.
- [x] Currently communicates in both HTTP requests and MQTT, standardise to just one (probably to HTTP and open the door or not based on the response code).
- [ ] Add error handling and reporting (maybe with leds?).
- [x] Clean functions with no side effects. ~not really 100% done but I like to see checkmarks~

## Desired Features:
- [ ] Values customisable in admin console: name, urls, delays.
- [ ] OTA updates.
- [ ] Manage cards and access on the admin console, then simplify the ESP's work to just read card and open door (the way it should've been from the start).
- [ ] Add a usable asset to the releases instead of just code.
- [ ] Explore other communication protocols that could be more appropriate for this use case.
- [ ] Ethernet if feasable for reliability and less processing.
- [ ] PoE too while we're at it.