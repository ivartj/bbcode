bbcode (unfinished)
======

C implementation of the BBCode markup used on many web forums. It only produces HTML.

Installation
------------

    $ git clone https://github.com/ivartj/bbcode
    $ cd bbcode
    $ autoreconf --install
    $ ./configure
    $ make all install clean

Usage
-----

    $ ed
    i
    This is [i]BBCode[/i].
    .
    wq file.txt
    $ bbcode file.txt
    This is <em>BBCode</em>.<br />

It also reads standard input if no file is given.

Why?
----

 * Nostalgia
 * It's a programming exercise
 * Markdown isn't any good for poetry (without adjustments)

Spec
----

(not all of this is implemented yet)

BBCodes have the following anatomies:

 * A start tag:

       [tag]

 * A start tag with a parameter:

       [tag=value]

 * A stop tag:

       [/tag]

 * A list item tag

       [*]

Registered tag names are divided into the following types:

 * Inline-level tags ([url], [b], [i])
 * Block-level tags ([quote], [list])
 * Content tags ([code], [img], [youtube])
 * Delimiter tags ([*])

Unmatched start and stop tags are printed verbatim.
Content tags alter how the source text between the tags is treated.
The [*] tag only has effect if the first block-level ancestor is a [list]; it is otherwise printed verbatim.

Inline-level tags can overlap with each other

    [b] test [i] test [/b] test [/i]

    <b> test <i> test </i></b><i> test </i>

Inline-level tags can overlap with block-level tags

    [b] test [quote] test [/b] test [/quote]

    <b> test </b><blockquote><b> test </b> test </blockquote>

Block-level tags can not overlap with each other

    [list] test [quote] test [/list] test [/quote]

    <ul> test [quote] test </ul> test [/quote]

They can still be nested

    [quote] test [list] test [/list] test [/quote]

    <blockquote> test <ul> test </ul> test </blockquote>


Operation
---------

The parser maintains a list of parsed tags and text content.

Any start tag is stored in the list immediately. When encountering stop tags the parser will attempt to find a previous matching start tag, and if found, it will set the `match` member of the two tags to 1. Start tags with `match` set to 1 can no longer be matched to any stop tags. Block-level stop tags can only be matched to block-level start tags in the same nesting (ignoring unviable unmatched block-level tags).

The printer maintains a doubly-linked lists of printed inline-level start tags and 'unwinds' and 'rewinds' it as needed to emulate overlapping formatting. It prints the source of unmatched tags verbatim.

TODO
----

Relativley complicated tasks include:

 * Lists

 * Ensure that XSS is not possible with stuff like [url=javascript:alert('hello')] from untrusted users

