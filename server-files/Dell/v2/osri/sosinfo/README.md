Different models retrieve different files in this directory using the
model identifier tag as the filename.

This allows an attacker to easily provide individual per-model payloads
and target all vulnerable models at once.

8891J63 is the Inspiron 3880
9WC4P93 is the Latitude 5320

Targeting any additional models will require adding new files here and
will likely require tuning the payload search addresses.

The text descriptions in the 9WC4P93 file still describe the Inspiron 3880
because we just copied the previously working json file when we started
working on adjusting the payload for the Latitude 5320.

