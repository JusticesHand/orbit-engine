# orbit-engine
Simple game "engine" (probably closely coupled with a game)

# Goals
As (yet another) reboot of the project, more things can be integrated. Knowledge acquired so far in past iterations are, in no particular order:
- OpenGL rendering (3.2+, not using fixed pipeline as shown in school)
- Vulkan rendering (Along with learning the whole graphics stack, as it is required to properly use Vulkan)
- (Simple) Composite tree to handle game objects. Factories to create said objects, loaded dynamically.
- Multithreaded application, separating concerns into different threads (i.e. rendering/input, world update, networking, etc)
- Networking

This iteration's goals are, again in no particular order:
- Integrate past knowledge into a more cohesive form
- Integrate new technologies with a better iteration approach (not "from-the-ground-up" as it has been so far)
- Separate tasks before jumping into them, get a more "planned" approach (issue tracking, design documents, etc)
- Use LunarG's cpp headers for Vulkan instead of its c headers

## Can I contribute?
No.

This is a personal project of sorts. You can use it as you wish (as it is under the MIT licence), but it serves as a learning platform for me.

## Can I use [parts of] this?
Yes. Nothing is guaranteed to work, however, so use it at your own risk.
