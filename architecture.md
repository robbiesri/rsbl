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

#### Hoisting window creation functionality to static free functions

#### Window Size vs Client Size

#### Confusing similarly named fields + parameters

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

#### Claude Opus 4.5 (Web UI)

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