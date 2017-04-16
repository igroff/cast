### Storing Stuff So You Can Find It

#### Storage Format

This is about storing things, including files, so the basic storage is handled
in a directory structure any fancy functionality can be in addition to this basic
filesystem layout.

The basic premise is keeping categories of things (files, bookmarks, web pages, etc.)
by the time of aquisition so that you can come back later and answer questions like
'What was that web page I saw last tuesday about horse saddles?' in a relatively 
easy way.

The structure of data is extensible and generally follows this pattern:

my-data-dir
  | - tasks
    | - 1-unique-id.task
    | - 2-unique-id.task
  | - completed
    | - 3-unique-id.task
  | - archive
    | - 2017
      | - 04
        | - 22
          | - monkey-pants.txt.mjson
          | - monkey-pants.txt
        | - 23
          | - sweet-vid.mov.mjson
          | - sweet-vid.mov
          | - some-data.json.mjson
          | - some-data.json
          | - rendered-page.pdf.mjson
          | - rendered-page.pdf

Items that are fully processed are put in the archive folder in a path structured
by the date the item was stored.

In the tasks folder are stored the files describing tasks that still need to be
performed such as downloading files, rendering web pages, etc.

Each file is stored with a 'buddy' \*.mjson file. This file contains all the info
supplied when the file was added, it's the metadata that goes along with the file.

