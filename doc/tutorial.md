# XPaF Tutorial

## Overview

In this tutorial, we demonstrate how to write a parser that extracts data from Twitter profile pages, showcasing various features of the XPaF parsing framework along the way.

## Getting started

Throughout this tutorial, we'll use a [snapshot of Bill Gates's Twitter page](https://github.com/google/xpaf/blob/master/src/testing/xpaf_bm_data/twitter_billgates.http) as our working example. Note that the [current version](https://twitter.com/BillGates) of this page looks somewhat different, so our parsers would need tweaking.

Before you proceed, we strongly encourage you to install the [XPath Helper Chrome extension][crx], which makes it easy to extract, edit, and evaluate XPath queries on any webpage. Then, as you make your way through this tutorial, you can use the extension to identify XPath queries for fields we're trying to extract, and then simplify those queries by editing them in the console. You can also take queries directly from the parsers below, paste them into the console, and see their evaluated results. (It's probably worth noting that we used XPath Helper extensively when writing this tutorial!)

If you want to run the parsers from this tutorial, you'll need to install XPaF. Linux and OS X installation instructions are available [here](https://github.com/google/xpaf/blob/master/README.md).

_Note: We use Bill Gates's Twitter page strictly as an example. We do not encourage activities that violate Twitter's terms of service._

## Extracting single fields

Let's start simple.

Suppose we just want to associate Bill's profile url with his name. The following parser does the trick:

    parser_name: "my_twitter_parser"

    relation_tmpls {
      subject: "%url%"
      predicate: "name"
      object: "//span[@class='fn']"

      subject_cardinality: ONE
      object_cardinality: ONE
    }

#### Result

    url: "http://twitter.com/billgates"
    parser_outputs {
      parser_name: "my_twitter_parser"
      relations {
        subject: "http://twitter.com/billgates"
        predicate: "name"
        object: "Bill Gates"
      }
    }

This parser consists of a name (`my_twitter_parser`) and a single RelationTemplate; the template associates Bill's url (`%url%` is replaced with the actual url) with the result of the XPath query `//span[@class='fn']`, which in our case evaluates to "Bill Gates". You may be wondering what `subject_cardinality` and `object_cardinality` are for; we'll get to that soon.

Extracting additional fields is simply a matter of adding RelationTemplates. For example, the following parser extracts Bill's photo in addition to his name:

    parser_name: "my_twitter_parser"
    url_regexp: "^http://twitter\\.com/"

    relation_tmpls {
      subject: "%url%"
      predicate: "name"
      object: "//span[@class='fn']"

      subject_cardinality: ONE
      object_cardinality: ONE
    }
    relation_tmpls {
      subject: "%url%"
      predicate: "photo"
      object: "//img[@id='profile-image']/@src"

      subject_cardinality: ONE
      object_cardinality: ONE
    }

#### Result

    url: "http://twitter.com/billgates"
    parser_outputs {
      parser_name: "my_twitter_parser"
      relations {
        subject: "http://twitter.com/billgates"
        predicate: "name"
        object: "Bill Gates"
      }
      relations {
        subject: "http://twitter.com/billgates"
        predicate: "photo"
        object: "http://a2.twimg.com/profile_images/486782671/Picture_18_bigger.png"
      }
    }

You may have noticed that we also specified `url_regexp` above. This instructs the parsing framework (more specifically, `XpafParserMaster`) to only run our parser on documents whose urls match the regular expression `^http://twitter\\.com/`. Specifying `url_regexp` offers two important benefits: (1) it prevents us from extracting bogus data from non-Twitter documents that happen to match our XPath queries, and (2) it dramatically improves performance in situations where most documents aren't relevant to our parser, since we can avoid running our parser on documents with non-matching urls.

## Extracting repeated fields

Okay, so extracting individual fields is easy... how about repeated fields? For example, suppose we wish to extract the set of users that Bill follows.

XPaF makes this equally simple:

    parser_name: "my_twitter_parser"
    url_regexp: "^http://twitter\\.com/"

    relation_tmpls {
      subject: "%url%"
      predicate: "follows"
      object: "//span[@class='vcard']/a[@class='url']/@href"

      subject_cardinality: ONE
      object_cardinality: MANY
    }

#### Result

    url: "http://twitter.com/billgates"
    parser_outputs {
      parser_name: "my_twitter_parser"
      relations {
        subject: "http://twitter.com/billgates"
        predicate: "follows"
        object: "http://twitter.com/PaulGAllen"
      }
      relations {
        subject: "http://twitter.com/billgates"
        predicate: "follows"
        object: "http://twitter.com/SteveCase"
      }

      /* ... skipping 32 relations ... */

      relations {
        subject: "http://twitter.com/billgates"
        predicate: "follows"
        object: "http://twitter.com/WFP"
      }
      relations {
        subject: "http://twitter.com/billgates"
        predicate: "follows"
        object: "http://twitter.com/GAVIAlliance"
      }
    }

Note that in this case, we set `subject_cardinality` to `ONE` and `object_cardinality` to `MANY`. This tells XPaF that we expect a one-to-many relationship between our subject (Bill's url) and objects (urls of users that Bill follows), and that the subject should be duplicated for each output relation.

In other cases, we may want to set both `subject_cardinality` and `object_cardinality` to `MANY`. For example, to associate each followee's profile url with his/her corresponding photo url, we'd write something like this:

    parser_name: "my_twitter_parser"
    url_regexp: "^http://twitter\\.com/"

    relation_tmpls {
      subject: "//span[@class='vcard']/a[@class='url']/@href"
      predicate: "photo"
      object: "//span[@class='vcard']//img[@class='photo fn']/@src"

      subject_cardinality: MANY
      object_cardinality: MANY
    }

#### Result

    url: "http://twitter.com/billgates"
    parser_outputs {
      parser_name: "my_twitter_parser"
      relations {
        subject: "http://twitter.com/PaulGAllen"
        predicate: "photo"
        object: "http://a1.twimg.com/profile_images/95218752/pallen_mini.jpg"
      }
      relations {
        subject: "http://twitter.com/SteveCase"
        predicate: "photo"
        object: "http://a2.twimg.com/profile_images/555579649/steve_case_wsj_mini.jpg"
      }

      /* ... skipping 32 relations ... */

      relations {
        subject: "http://twitter.com/WFP"
        predicate: "photo"
        object: "http://a3.twimg.com/profile_images/899668508/twitter_profile_03_mini.png"
      }
      relations {
        subject: "http://twitter.com/GAVIAlliance"
        predicate: "photo"
        object: "http://a3.twimg.com/profile_images/1168422796/5e7534c3-531d-45f4-a69b-589656027c2c_mini.gif"
      }
    }

Note: For many-to-many relations, if the number of extracted subjects doesn't equal the number of extracted objects, we output zero relations (since we don't know which subject to associate with which object). QueryGroupDefs, described later in this tutorial, are designed to better handle such situations.

## QueryDefs

Armed just with RelationTemplates, we can already write some pretty nice parsers. But suppose we wish to reuse the same XPath query in multiple RelationTemplates, e.g. combining our two previous parsers, both of which use the query `//span[@class='vcard']/a[@class='url']/@href`. QueryDefs make this easy.

A QueryDef defines a query to be referenced by one or more RelationTemplates -- think of it as a variable. In its simplest form, a QueryDef consists of a name and a query. For example, the following parser uses a QueryDef named `following_url` to combine our previous two parsers without repeating their overlapping XPath query.

    parser_name: "my_twitter_parser"
    url_regexp: "^http://twitter\\.com/"

    query_defs {
      name: "following_url"
      query: "//span[@class='vcard']/a[@class='url']/@href"
    }

    relation_tmpls {
      subject: "%url%"
      predicate: "follows"
      object: "%following_url%"

      subject_cardinality: ONE
      object_cardinality: MANY
    }
    relation_tmpls {
      subject: "%following_url%"
      predicate: "photo"
      object: "//span[@class='vcard']//img[@class='photo fn']/@src"

      subject_cardinality: MANY
      object_cardinality: MANY
    }

#### Result

    url: "http://twitter.com/billgates"
    parser_outputs {
      parser_name: "my_twitter_parser"
      relations {
        subject: "http://twitter.com/billgates"
        predicate: "follows"
        object: "http://twitter.com/PaulGAllen"
      }
      relations {
        subject: "http://twitter.com/billgates"
        predicate: "follows"
        object: "http://twitter.com/SteveCase"
      }

      /* ... skipping 32 relations ... */

      relations {
        subject: "http://twitter.com/billgates"
        predicate: "follows"
        object: "http://twitter.com/WFP"
      }
      relations {
        subject: "http://twitter.com/billgates"
        predicate: "follows"
        object: "http://twitter.com/GAVIAlliance"
      }
      relations {
        subject: "http://twitter.com/PaulGAllen"
        predicate: "photo"
        object: "http://a1.twimg.com/profile_images/95218752/pallen_mini.jpg"
      }
      relations {
        subject: "http://twitter.com/SteveCase"
        predicate: "photo"
        object: "http://a2.twimg.com/profile_images/555579649/steve_case_wsj_mini.jpg"
      }

      /* ... skipping 32 relations ... */

      relations {
        subject: "http://twitter.com/WFP"
        predicate: "photo"
        object: "http://a3.twimg.com/profile_images/899668508/twitter_profile_03_mini.png"
      }
      relations {
        subject: "http://twitter.com/GAVIAlliance"
        predicate: "photo"
        object: "http://a3.twimg.com/profile_images/1168422796/5e7534c3-531d-45f4-a69b-589656027c2c_mini.gif"
      }
    }

Beyond just giving us a way to reuse XPath queries, QueryDefs also enable us to run custom post-processing operations (such as find-replace) on the raw XPath query results. This functionality is described towards the end of the tutorial.

## QueryGroupDefs

The astute reader may have noticed a potential problem with our parser that associates followees' urls with their photos: What if one of Bill's followees has a photo but no url? In this case, our XPath query for photos would return 36 results, but our XPath query for urls would return 35 results, and our poor XPaF parser wouldn't know how to associate photos with urls.

As noted previously, XPaF handles this situation as best it can -- it outputs zero relations. But now consider a far more dire scenario: Suppose Bill's first followee has a photo but no url, and his second followee has a url but no photo. In this case, XPath would happily return 35 results for each of our queries, and we'd incorrectly associate the first followee's photo with the second's url. Yikes!

QueryGroupDefs are designed specifically to deal with this situation. A QueryGroupDef defines a group of queries that share a DOM root node; it consists of a name, a "root query" that identifies root nodes, and a list of QueryDefs with queries relative to the root query. The gory details can be found [here][xpaf_parser_def] (search for `QueryGroupDef`), but for the purposes of this tutorial a quick example should suffice:

    parser_name: "my_twitter_parser"
    url_regexp: "^http://twitter\\.com/"

    query_group_defs {
      name: "following"

      root_query: "//span[@class='vcard']"

      query_defs {
        name: "url"
        query: "/a[@class='url']/@href"
      }
      query_defs {
        name: "photo"
        query: "/a/img[@class='photo fn']/@src"
      }
    }

    relation_tmpls {
      subject: "%url%"
      predicate: "follows"
      object: "%following.url%"

      subject_cardinality: ONE
      object_cardinality: MANY
    }
    relation_tmpls {
      subject: "%following.url%"
      predicate: "photo"
      object: "%following.photo%"

      subject_cardinality: MANY
      object_cardinality: MANY
    }

#### Result

    url: "http://twitter.com/billgates"
    parser_outputs {
      parser_name: "my_twitter_parser"
      relations {
        subject: "http://twitter.com/billgates"
        predicate: "follows"
        object: "http://twitter.com/PaulGAllen"
      }
      relations {
        subject: "http://twitter.com/billgates"
        predicate: "follows"
        object: "http://twitter.com/SteveCase"
      }

      /* ... skipping 32 relations ... */

      relations {
        subject: "http://twitter.com/billgates"
        predicate: "follows"
        object: "http://twitter.com/WFP"
      }
      relations {
        subject: "http://twitter.com/billgates"
        predicate: "follows"
        object: "http://twitter.com/GAVIAlliance"
      }
      relations {
        subject: "http://twitter.com/PaulGAllen"
        predicate: "photo"
        object: "http://a1.twimg.com/profile_images/95218752/pallen_mini.jpg"
      }
      relations {
        subject: "http://twitter.com/SteveCase"
        predicate: "photo"
        object: "http://a2.twimg.com/profile_images/555579649/steve_case_wsj_mini.jpg"
      }

      /* ... skipping 32 relations ... */

      relations {
        subject: "http://twitter.com/WFP"
        predicate: "photo"
        object: "http://a3.twimg.com/profile_images/899668508/twitter_profile_03_mini.png"
      }
      relations {
        subject: "http://twitter.com/GAVIAlliance"
        predicate: "photo"
        object: "http://a3.twimg.com/profile_images/1168422796/5e7534c3-531d-45f4-a69b-589656027c2c_mini.gif"
      }
    }

When XPaF runs this parser, it first issues the XPath query `//span[@class='vcard']` to identify root nodes. Then, it issues each QueryDef's query (appended to the root query) and associates results that share the same root node. In other words, it issues `//span[@class='vcard']/a[@class='url']/@href` and `//span[@class='vcard']/a/img[@class='photo fn']/@src` and then associates urls and photos that share the same `//span[@class='vcard']` node.

For the problematic example described above, where one followee has no url, another has no photo, and the remaining 34 have both, a QueryGroupDef that associates urls with photos would yield 34 output pairings, all correct.

## Annotations

Beyond simple triples (subject-predicate-object), XPaF also supports annotating relations with additional data using AnnotationTemplates.

For example, to extract Bill's tweets and annotate them with timestamps and recipient counts, we could write the following:

    parser_name: "my_twitter_parser"
    url_regexp: "^http://twitter\\.com/"

    query_group_defs {
      name: "tweet"

      root_query: "//li[contains(@class, 'status')]"

      query_defs {
        name: "content"
        query: "//span[@class='entry-content']"
      }
      query_defs {
        name: "timestamp"
        query: "//span[@class='published timestamp']"
      }
    }

    relation_tmpls {
      subject: "%url%"
      predicate: "tweeted"
      object: "%tweet.content%"

      subject_cardinality: ONE
      object_cardinality: MANY

      annotation_tmpls {
        name: "timestamp"
        value: "%tweet.timestamp%"
        value_cardinality: MANY
      }
      annotation_tmpls {
        name: "num_recipients"
        value: "//span[@id='follower_count']"
        value_cardinality: ONE
      }
    }

#### Result

    url: "http://twitter.com/billgates"
    parser_outputs {
      parser_name: "my_twitter_parser"
      relations {
        subject: "http://twitter.com/billgates"
        predicate: "tweeted"
        object: "A fun interview Melinda & I did with Warren from India - http://bit.ly/hOPDuH - an incredible trip, thanks to everyone who participated..."
        annotations {
          name: "timestamp"
          value: "7:41 AM Mar 30th"
        }
        annotations {
          name: "num_recipients"
          value: "2,407,290 "
        }
      }
      relations {
        subject: "http://twitter.com/billgates"
        predicate: "tweeted"
        object: "Today is World TB day \342\200\223 http://bit.ly/fEuSNS - more on some great work and innovation we saw here in India at the L.R.S. Institute"
        annotations {
          name: "timestamp"
          value: "1:09 AM Mar 24th"
        }
        annotations {
          name: "num_recipients"
          value: "2,407,290 "
        }
      }

      /* ... skipping 16 relations ... */

      relations {
        subject: "http://twitter.com/billgates"
        predicate: "tweeted"
        object: "Congratulations to my dear friend Warren Buffett on National Medal of Freedom - http://bit.ly/eN9rh1"
        annotations {
          name: "timestamp"
          value: "2:12 PM Feb 15th"
        }
        annotations {
          name: "num_recipients"
          value: "2,407,290 "
        }
      }
      relations {
        subject: "http://twitter.com/billgates"
        predicate: "tweeted"
        object: "A nice review of my friend Nathan Myhrvold\'s cookbook which is really quite good - http://on.wsj.com/f0K5ET"
        annotations {
          name: "timestamp"
          value: "4:23 PM Feb 12th"
        }
        annotations {
          name: "num_recipients"
          value: "2,407,290 "
        }
      }
    }

Like subjects and objects, annotation values have associated cardinalities; in our case, we used cardinality MANY for `timestamp` and cardinality ONE for `num_recipients`, causing the latter to be duplicated for each output relation.

If the number of results returned for a given annotation value disagrees with any cardinalities or result counts, the AnnotationTemplate is skipped, i.e. no annotations are emitted for that template. (On the other hand, for subject and object cardinality errors, the entire RelationTemplate is skipped.)

## Advanced features and extensions

RelationTemplates, AnnotationTemplates, QueryDefs, and QueryGroupDefs together define the core of XPaF. Users of XPaF often run application-specific post-processing operations on the [ParsedDocument][parsed_document] protocol buffers produced by XPaF. However, to better support common use cases in document parsing, XPaF also provides several generic filtering, post-processing, and extension mechanisms, described in this section.

<!-- TODO(sadovsky): Maybe move some of this content to the FAQ? -->

#### Running multiple parsers

In this tutorial, all of our examples have consisted of single parsers. However, it's just as easy to define and run multiple parsers -- the `XpafParserDefs` object passed to `XpafParserMaster` may contain multiple `XpafParserDef` objects, and `ParsedDocument` may contain multiple `ParserOutput` objects.

#### QueryDef post-processing operations

TODO(sadovsky): Fill this in.

#### Filtering operations

TODO(sadovsky): Fill this in.

#### Extension support

To facilitate application-specific business logic, XPaF allows parser authors to embed arbitrary string data in `userdata` fields of the `XpafParserDef` and `RelationTemplate` protocol buffers. Also, `XpafParserMaster` itself should be thought of as a reference implementation; it can be replaced (and `XPafParserDef` can be embedded) to allow for additional customizations.

All of these features are described in more detail in the [XPaF parser specification][xpaf_parser_def].

## Resources

Hopefully this tutorial has given you a flavor for what's possible with XPaF. However, in the interest of brevity, plenty of details were omitted; the full XPaF parser specification is available [here][xpaf_parser_def]. For the adventurous, a brief stroll through the [XPaF implementation](https://github.com/google/xpaf/tree/master/src) is highly recommended (start with `xpaf_parser.h`).

More examples of XPaF parsers can be found in the [unit test directory](https://github.com/google/xpaf/tree/master/src/testing/parse_test_data). These tests are (or ought to be) comprehensive; everything described in this tutorial should have an associated test.

We also recommend reading through the [FAQ](faq.md) and installing the [XPath Helper Chrome extension][crx], if you haven't done so already.

Finally, please do not hesitate to contact the author with any questions or suggestions.

[crx]: https://chrome.google.com/webstore/detail/xpath-helper/hgimnogjllphhhkhlmebbmlgjoejdpjl
[parsed_document]: https://github.com/google/xpaf/blob/master/src/parsed_document.proto
[xpaf_parser_def]: https://github.com/google/xpaf/blob/master/src/xpaf_parser_def.proto
