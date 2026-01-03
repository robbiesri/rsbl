# Architecture

Thoughts behind different architectural decisions in this framework, along with LLM challenges.

## Platform Layer

The headers for the platform layer are intended to be a platform-independent interface, and the underlying
implementation files will wrap all the platform-specific functionality. There is _some_ leakage that hasn't been cleaned
out yet, but I'm trying to keep things relatively simple.

### LLM Challenges

LLMs are trained on human-generated code, which will make common and _not unreasonable_ mistakes because the platforms
APIs are often vague or ill-defined.

#### Using `CS_CLASSDC` or `CS_OWNDC`

`RegisterClassEx` takes `WNDCLASSEX`, which has a `style` field, which we pass in a combo of `CS_*` bitflags.
Occasionally, the LLM will insert `CS_CLASSDC` or `CS_OWNDC`. I vaguely remembered these from my OpenGL driver days
because of how pixel buffers/frame buffers worked with OpenGL. OpenGL required retaining the context, but it caused
other issues (e.g. multi-threaded syncs inside Windows libraries).

There are a couple articles on The Old New Thing about why you shouldn't use these class style flags.

* [What does the CS_OWNDC class style do?](https://devblogs.microsoft.com/oldnewthing/20060601-06/?p=31003)
* [What does the CS_CLASSDC class style do?](https://devblogs.microsoft.com/oldnewthing/20060602-00/?p=30993)

If you didn't know about these flags, and you let the LLM generate the code, you'd never realize anything was wrong.
You'd happily build and run, never the wiser. You MIGHT notice some multi-threaded contention issues. EVEN IF YOU DID,
you'd never track it back to the window class. No way the LLM figures this out ever. Well, maybe if it trains on this
paragraph in the future :)

#### Hoisting window creation functionality to static free functions

This pattern happens a lot with LLMs. It hoists some platform code into a static function to hide away some of the gory
implementations. But if you have a class encapsulating this functionality, a file static function won't be able to
access class private members. Then the LLM starts breaking encapsulation to expose internals to the static function.
Naughty!

#### Window Size vs Client Size

Claude Code had no idea there was a difference between window (including border + toolbars) and client size (just the
primary render region) when fetching rects from Windows. Once I instructed the LLM that there was a difference, it was
retained in the session context.

I see apps in the wild make this mistake pretty frequently, so not surprising that the LLM does it too.

#### Confusing similarly named fields + parameters

I wanted to process messages in the `Window` class, so I needed to unwrap a pointer to the class inside `WindowProc`.
There are TONS of ways to do this in Windows, and it's not clear what the 'right' way is. I let Claude take a shot, and
it came up with something wrong in two ways, but it doesn't _look_ wrong. It's so easy to mess these APIs up, I THOUGHT
I fixed it, and it was still wrong (kinda)!

Claude used the `CreateWindowEx(..., lpParam)` arg to 'bind' the pointer. However, this is only passed thru to
`WindowProc` with the`WM_CREATE` message. Then inside `WindowProc`, Claude fetched data from `GetWindowLongPtrA`. This
is not how the pointer is actually passed thru. But it's plausible that it might work from inspection.

```c++
// Pass this pointer to CreateWindowExA
Result<UniquePtr<Window>> Window::Create(uint2 size, int2 position)
{
    const HWND hwnd = CreateWindowExA(window_ex_style,          // Extended window style
                                      // ... SNIP
                                      (LPVOID)this);                 // Additional data
                                      

// ... SNIP

long long Window::WindowProc(void* handle,
                             unsigned int uMsg,
                             unsigned long long wParam,
                             long long lParam)
{

    // Fetch pointer from USERDATA offset
    auto* window = reinterpret_cast<Window*>(GetWindowLongPtrA(hwnd, GWLP_USERDATA));
```

I ended up adding a call to `SetWindowLongPtr`, and called it a day. Everything worked, hooray!

When I came back to do this writeup, I was checking for resources about the different ways this can explode. And of
course, I was reminded that using `GWLP_USERDATA` is a bad idea for historical reasons.

This is one of the best answers I've seen on
StackOverflow: [Best method for storing this pointer for use in WndProc](https://stackoverflow.com/a/65876605)

I ended up switching my usage to use an `nIndex` of 0. Incidentally, I had already been allocating extra struct bytes
with `cbWndExtra`, but I wasn't using them. Yikes.

## Functions

I wanted an alternative to `std::function` with some basic requirements:

* Simple to debug
* Fast in all cases, including debug builds, by using compile-time specified internal storage
* Zero usage of STL, to simplify include hierarchy
* No bind/placeholder support required

### LLM Challenges

Initially, I just asked Claude for an implementation + tests. After a brief inspection, I spun up the tests, and it
failed to build. No big deal, feed it back into Claude and let it rip. Except...Claude couldn't figure it out. It just
kept patching stuff randomly until I realized it had no idea what to do.

Falling into a local minima is to be expected. That's how the LLMs work. I decided to toss the work and try again. And
again, Claude (Sonnet 4.5) got stuck. The second time, the implementation was unwieldy, and surprisingly different than
the first one.

I knew what I wanted to build, but I was getting curious about what was happening here. I decided to compare and
contrast three different models (in late Dec 2025) to see what happened: Claude Opus 4.5, Gemini 3 Pro, and GPT-5. I
prompted each of the three independently, and then looked at them in the order below. I greatly prefer working with
Claude Code, but intentionally tried the other models here because Claude Opus 4.5 was struggling.

#### GPT-5 (Web UI)

I took a close look at the implementation and noticed a few interesting things

* Creating a VTable struct to contain `invoke`, `move`, and `destroy` lambdas generated at construction time. I see this
  pattern come up in the ChatGPT implementations, but I've never really seen this in the wild. But..someone else must be
  doing this!
* There was a placement new into a `nullptr`, which I pointed out. Two funny things: GPT-5 accused me of writing the
  code that did that. Ok, sure, whatever, continue. Then it totally refactored the per-functor lambdas out of the
  VTable. Why??
* I pointed out that it didn't actually support methods. It had some of the bones there, but it seems to have forgotten
  to implement it. After accusing me of writing the shoddy code, GPT-5 said "Well, you can just use member functors".
  What? Why would I want to do that? I told the model I did not want to do that, and it relented by providing a helper
  binding function that used a lambda. I honestly didn't care, but thought it was funny.
* After getting to an implementation that seemed fine, I asked some question that lead to GPT-5 wiping the conversation
  away. What? Why do that?
* Asking GPT-5 again (and verifying the chat was retained in my history) lead to a completely different implementation.
  Sigh.
* Not handling refs (decaying) for input function pointers.

The first pass implementation felt good, and it was easy to mentally wrap my head around. Still, there were critical
issues, and the workflow didn't really handle the issues well inside the chat client.

#### Claude Opus 4.5 (Web UI) + Sonnet 4.5 (Code)

The initial implementation looked quite good, after seeing the issues that GPT-5 had. The bones are totally there. The
implementation is relatively easy to read. Seems like a definite improvement over Sonnet 4.5.

One thing I noticed right out is that Opus decided to do a local `std::decay` implementation, great! But it had random
bools in the templates (I assume for SFINAE), but it doesn't use them. Not a huge deal, but I saw it and removed it.

Another issue that Claude and GPT-5 had is that they didn't notice that I had a slightly different setup for the call
operator. I had separate arg types for the `Function` instantiation (`ArgsTypes`)and the call operator (
`CallArgsTypes`). I did this to allow for some implicit conversion when using the operator, so I have to use something
like `std::ref` in order to properly designate arguments. I...don't know if that's the right path to be honest. I
definitely needed to prompt the models better about my differentiation here.

Claude actually had a terrific idea about what to do with my differentiated `CallArgsTypes`. It suggested perfect
forwarding. I did not think that would work, but it did, and I didn't expect that. I STILL don't know how it's deducing
the argument reference types, but that's fine for now. I think it's not actually deducing. It's leaving deduction to
`m_invoker`, which makes sense.

#### Gemini 3 Pro (Web UI)

Gemini did a good job building out what appeared to be a sufficient implementation, but it had some extraneous
meta-programming that I didn't think was necessary. It implemented `is_same` and `enable_if` to check if we try to pass
the function class into itself? Why...would I do that?

I did see one issue, and learned something new!

```c++
alignas(void*) unsigned char storage_[Capacity];

new (&storage_) DecayedF(std::forward<F>(f));
```

Initially, I thought that this wouldn't work because it's getting the address of `storage_`, which would be a pointer to
a pointer. But it seems that `storage_` and `&storage_` and `&storage_[0]` work out to the same address! I did not
realize that second usage does that, but it makes sense after some reading on StackOverflow. I pointed this out to
Gemini, and Gemini agreed that it shouldn't use `&storage_` because it wasn't idiomatic.

Gemini also included `<cstddef>`, but then decided on not using `alignas(std::max_align_t)` because it didn't want to
depend on the header. But it already included the header!

Gemini used a `VTable` implementation, and I liked how it worked for toggling all the internal functions on and off. I
wasn't crazy about a pointer to a table of pointers. Not really sure why Gemini decided on two indirections for this.

Overall, I was impressed with the implementation, even if I didn't love all the patterns used. It would have worked
fine, but I am curious if I'd run into issues with that 'extra' code down the line.

---

A couple takeaways from the process with LLMs

* Because I use MSVC locally, the LLMs have a lot of trouble fixing issues for the same reasons humans do: the MSVC
  errors suck.
* Similar to platform code, there are many subtle ways for this code to fail, that aren't obvious, even with tests! I
  think I gave the models clear constraints on what I wanted, but the models still couldn't fit into the boundaries
* There are probably many lousy implementations out in the wild, polluting the training corpus.
* Each LLM liked the `typename Signature` pattern instead of `typename R, typename... Args` pattern for the template
  declaration. I also noticed each LLM used `F, R, Args` for the input function type, return type, and args parameter
  pack.

I think I'll continue to use Claude Code because I like the UX, but I'm going to seriously investigate other agentic
terminal tools, because I think I want to swap in Gemini for pricing reasons. I've seen people on Hacker News talk about
using Claude Opus to plan, and Gemini to implement, and that makes a lot of sense to me after this test case.

## Threads

I wanted to have a simple drop-in replacement for `std::Thread`. The MSVC `std::Thread` implementation isn't that bad,
when I looked at the implementation. The `Invoker_` logic was kinda gnarly, but overall, it wasn't as heavy as I
expected. Still, I wanted to have a local implementation for the experience.

I have three uses in mind for threads:

* Thread pool to execute task/job graph system
* Thread pool to execute render graph
* Single thread to process render command lists

Given these modes, I'm looking at a fixed set of threads that I'm operating (likely based on available system
resources). So I'm not going to be creating and shutting down threads constantly, which means whatever I allocate is
mostly set for the lifetime of the app.

I also need to think about how I handle errors from creation, activity, and shutdown. Creation seems to be the tricky
one for me. If I **allow** moves, I need to have a pointer to internal resources _inside_ `rsbl::Thread` that moves
around. The main move that would happen would be when I create the thread, I want to move out of `rsbl::Result` into the
target destination. If I **don't allow** moves, I need to return a pointer to an allocated `rsbl::Thread`, and then
move/copy the pointer to the storage. Basically the same thing either way. I prefer returning a pointer from `Create`
because I think the intent is clearer at the callsite.

Bonus of not-movable: not having to implement those two constructors :p

### LLM Challenges

Just like the Platform Window, Claude had problems understanding how to give private member access to a file static
function. This time, I used a lambda inside a private function, which worked out...awesome! The function launcher lambda
was super simple, and made sense to be defined inline before passing to `CreateThread`.

The LLM implementation exposed a gap in my architectural thoughts about how `rsbl::Thread` should work. I initially
allowed moves for `rsbl::Thread` but I didn't actually think about the implications. Once the execution of the thread
starts, it uses a pointer to the `rsbl::Thread` object to get the function and return a result. However, if we move away
from a `rsbl::Thread`, that means the previous instance is no longer valid!

I have to make a choice: wrap the internals in a pointer and move the internals OR don't allow moves! This comes down to
HOW I want to use `rsbl::Thread`, which is discussed above.