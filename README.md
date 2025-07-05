# Bootsmann
A minimalistic, lean and fast HTTP REST API test application (opensource C++ alternative to Postman/Insomnia/Hoppscotch for simple HTTP REST API testing).
It is especially useful for embedded devices and local development, where low memory usage and fast response times are crucial.

![image](https://github.com/user-attachments/assets/9f43498b-016d-4178-962f-4eae6ed43bef)

## Key Features
- **Lean**: Designed to be lightweight - low memory footprint and resource usage.
- **Native code**: Built with a C++ compiler - no Electron or other web-based frameworks used.
- **Direct calls**: Uses direct system calls to run the requests at highest speed - no JS-based middleware, therefore fast and efficient.
- **Offline-capable**: Can be used without an internet connection, making it suitable for local development and testing, especially for embedded devices.
- **Secure**: Does not upload any user data or API keys, ensuring privacy and security. No registration or login required.
- **Portable**: Can be run directly from the executable without installation, making it portable and easier to use.
- **Multiplatform**: Runs on various operating systems including Windows, macOS, and Linux - thanks to Qt framework.
- **Intuitive**: Provides a simple (but functional) graphical user interface.
- **Open source**: Available under the MIT License, allowing for free use, modification, and distribution.

<p align="center">
<img src="https://github.com/user-attachments/assets/b2a53b30-249e-4b5b-bd82-32bfbc65fe14" alt="HTTP" width="100">
<img src="https://github.com/user-attachments/assets/83787c03-e304-44cd-ba78-6a07ebe14c1f" alt="C++" width="100">
<img src="https://github.com/user-attachments/assets/22eb374a-8e25-4265-954d-e86a0e415578" alt="Qt" width="100">
</p>

## Core Functionality
- Building and executing HTTP requests visually
- Visual editing of parameters and headers
- Common file and clipboard operations
- Basic user authentification (Unencrypted, Basic, Bearer token)
- Parsing and visualising server replies in a human-convenient form
- Dynamic loading of HTML replies

## Measurements

| Average values            | Postman & Insomnia    | Hoppscotch	| Bootsmann
| --------------------------| --------------------- |-----------	|--------------- 
| Memory usage				| 400-500 MB			| 60-70 MB		| 20-40 MB
| Responce timings			| 120-150 ms			| 100-110 ms	| 70-80 ms

(Measurements are done on the same hardware and OS, running the same REST API requests. Values are averages of 10 runs.)

## Support
Since **Bootsmann** is a free software, it is developed in my spare time entirely at my own expense.
If you like the software, you can support its development by making a donation (one-time or monthly) via the button below:

  [![paypal](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=Z35EHHJ3729GG&source=url)

Thank you!
