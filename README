This is a Qt based daemon process to handle RP Lidar input and propagate it as a TCP server

The server produces a blob of data on completion of each scan (0 to 360 degrees).

The data is in the format:
  <start marker '!!RP!!'><number of vectors (ushort)><sequence number (ushort)><range (double) ... for the 'number of vectors'>

By default, the number of vectors is 1440, which is a range at each increment of .25°. 
So, the first vector contains the range at 0°, the second as .25° and so on to 359.75°.

The systemd service unit and the default config file are at the root of the repository, 
The service unit should be placed at /lib/systemd/system, and the config file at /etc/lidar/lidar.conf



