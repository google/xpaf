# XPath-based Parsing Framework (XPaF)

## Description

XPath-based Parsing Framework (XPaF) is a simple, fast, [open-source](https://github.com/google/xpaf) parsing framework that makes it easy to extract relations (subject-predicate-object triples) from HTML and XML documents.

For a quick example, see below. When you're ready for more, check out the [tutorial] or the [FAQ](faq.md).

## Installation

Linux and OS X installation instructions are available [here](https://github.com/google/xpaf/blob/master/README.md).

## Quick example

Consider the following HTML snippet:

    <table>
      <tr>
        <td class="name">Aaron</td>
        <td class="occ">Engineer</td>
      </tr>
      <tr>
        <td class="name">Jennifer</td>
        <td class="occ">Archeologist</td>
      </tr>
    </table>

To extract name-occupation pairs from such tables, we'd write the following XPaF parser:

    parser_name: "my_parser"
    relation_tmpls {
      subject: "//td[@class='name']"
      predicate: "occupation"
      object: "//td[@class='occ']"

      subject_cardinality: MANY
      object_cardinality: MANY
    }

To run our parser, we'd write the following C++ code:

    XpafParserDefs parser_defs;
    ReadXpafParserDefs("/path/to/my_parser.xpd", &parser_defs);
    ParseOptions parse_options;
    const XpafParserMaster master(parser_defs, parse_options);
    ParsedDocument output;
    master.ParseDocument(doc, &output);

For the HTML snippet and parser above, XPaF would produce the following output, encoded as a [ParsedDocument][parsed_document] protocol buffer object:

    parser_name: "my_parser"
    relations {
      subject: "Aaron"
      predicate: "occupation"
      object: "Engineer"
    }
    relations {
      subject: "Jennifer"
      predicate: "occupation"
      object: "Archeologist"
    }

For a more detailed example, check out the [tutorial].

The full parser specification language can be found [here][xpaf_parser_def] (start with `XPafParserDef`).

## Chrome extension

The [XPath Helper Chrome extension][crx] makes it easy to extract, edit, and evaluate XPath queries on any webpage. It's very helpful for writing XPaF parsers.

The source code for XPath Helper is available [here](https://github.com/google/xpaf/tree/master/chrome).

[crx]: https://chrome.google.com/webstore/detail/xpath-helper/hgimnogjllphhhkhlmebbmlgjoejdpjl
[parsed_document]: https://github.com/google/xpaf/blob/master/src/parsed_document.proto
[tutorial]: tutorial.md
[xpaf_parser_def]: https://github.com/google/xpaf/blob/master/src/xpaf_parser_def.proto
