# DEVELOPMENT / DEBUG Stuff

You can run the game with:

`./tbftss -debug`

to access some (rather crude) debugging and development stuff, activated by pressing certain keys on the keyboard. Press the following keys to toggle the states (see the console for output). Note to packagers - you shouldn't distribute the game in this state (either as a hard compile or run script), as it could result in unexpected behaviour.

* [1] - Make the player immortal
* [2] - Unlimited missiles
* [3] - Prevent the AI from using weapons
* [4] - Prevent the AI from taking any further actions
* [5] - Make everything immortal
* [6] - Immediately complete the current mission

* [9] - Show the current frames per second
* [0] - Take a screenshot once per second, saving to /tmp/tbftss on Linux.

By default, the game will output WARN level messages, or greater. To see INFO level messages, you can use:

`./tbftss -info`

Note that using `-debug` will automatically output DEBUG level messages.
