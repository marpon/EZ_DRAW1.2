#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# EZ-Draw documentation build configuration file.
#
# This file is exec()d from doc/src-*/conf.py

import sys
import os

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#sys.path.insert(0, os.path.abspath('.'))

# -- General configuration ------------------------------------------------

# If your documentation needs a minimal Sphinx version, state it here.
#needs_sphinx = '1.0'

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = []

# Add any paths that contain templates here, relative to this directory.
templates_path = ['templates']

# The suffix of source filenames.
source_suffix = '.rst'

# The encoding of source files.
#source_encoding = 'utf-8-sig'

# The master toctree document.
master_doc = 'ez-manual'

# General information about the project.
project = 'EZ-Draw'
copyright = '2014, Edouard Thiel'

# The version info for the project you're documenting, acts as replacement for
# |version| and |release|, also used in various other places throughout the
# built documents.
#
# The short X.Y version.
version = '1.2'
# The full version, including alpha/beta/rc tags.
release = '1.2'

# There are two options for replacing |today|: either, you set today to some
# non-false value, then it is used:
#today = ''
# Else, today_fmt is used as the format for a strftime call.
#today_fmt = '%B %d, %Y'

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
#exclude_patterns = ['build']

# The reST default role (used for this markup: `text`) to use for all
# documents.
#default_role = None

# If true, '()' will be appended to :func: etc. cross-reference text.
#add_function_parentheses = True

# If true, the current module name will be prepended to all description
# unit titles (such as .. function::).
#add_module_names = True

# If true, sectionauthor and moduleauthor directives will be shown in the
# output. They are ignored by default.
#show_authors = False

# The name of the Pygments (syntax highlighting) style to use.
pygments_style = 'sphinx'

# A list of ignored prefixes for module index sorting.
#modindex_common_prefix = []

# If true, keep warnings as "system message" paragraphs in the built documents.
#keep_warnings = False

# The name of the default domain.
primary_domain = 'c'

# The default language to highlight source code in.
highlight_language = 'none'


# -- Options for HTML output ----------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes. See http://sphinx-doc.org/theming.html
html_theme = 'default'

# Theme options are theme-specific and customize the look and feel of a theme
# further.  For a list of options available for each theme, see the
# documentation.
#html_theme_options = {}
html_theme_options = { 
    'stickysidebar'    : 'true',
    'footerbgcolor'    : '#FFF',
    'footertextcolor'  : '#555',
    'sidebarbgcolor'   : '#FFF',
    'sidebartextcolor' : '#0015A0',
    'sidebarlinkcolor' : '#002BB8',
    'relbarbgcolor'    : '#F0F0F0',
    'relbartextcolor'  : '#555',
    'relbarlinkcolor'  : '#000',
#   'bgcolor'          : 'red',
#   'textcolor'        : 'red',
    'linkcolor'        : '#002BB8',
    'visitedlinkcolor' : '#002BB8',
    'headbgcolor'      : '#FFF',
    'headtextcolor'    : '#000',
#   'headlinkcolor'    : 'red',
#   'codebgcolor'      : 'red',
#   'codetextcolor'    : 'red',
}

# Add any paths that contain custom themes here, relative to this directory.
#html_theme_path = []

# The name of an image file (relative to this directory) to place at the top
# of the sidebar.
#html_logo = None

# The name of an image file (within the static path) to use as favicon of the
# docs.  This file should be a Windows icon file (.ico) being 16x16 or 32x32
# pixels large.
#html_favicon = None

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ['static']

# Add any extra paths that contain custom files (such as robots.txt or
# .htaccess) here, relative to this directory. These files are copied
# directly to the root of the documentation.
#html_extra_path = []

# If not '', a 'Last updated on:' timestamp is inserted at every page bottom,
# using the given strftime format.
#html_last_updated_fmt = '%b %d, %Y'

# If true, SmartyPants will be used to convert quotes and dashes to
# typographically correct entities.
#html_use_smartypants = True

# Custom sidebar templates, maps document names to template names.
html_sidebars = {
   '**': ['quicklinks.html','globaltoc.html', 'indexlink.html', 'searchbox.html'],
}

# Additional templates that should be rendered to pages, maps page names to
# template names.
#html_additional_pages = {}

# If false, no module index is generated.
#html_domain_indices = True

# If false, no index is generated.
#html_use_index = True

# If true, the index is split into individual pages for each letter.
#html_split_index = False

# If true, links to the reST sources are added to the pages.
html_show_sourcelink = False

# If true, "Created using Sphinx" is shown in the HTML footer. Default is True.
#html_show_sphinx = True

# If true, "(C) Copyright ..." is shown in the HTML footer. Default is True.
#html_show_copyright = True

# If true, an OpenSearch description file will be output, and all pages will
# contain a <link> tag referring to it.  The value of this option must be the
# base URL from which the finished HTML is served.
#html_use_opensearch = ''

# This is the file name suffix for HTML files (e.g. ".xhtml").
#html_file_suffix = None

# Output file base name for HTML help builder.
htmlhelp_basename = 'EZ-Draw-doc'


# -- Options for LaTeX output ---------------------------------------------

latex_elements = {
# The paper size ('letterpaper' or 'a4paper').
#'papersize': 'letterpaper',

# The font size ('10pt', '11pt' or '12pt').
#'pointsize': '10pt',

# Additional stuff for the LaTeX preamble.
#'preamble': '',
}

# Grouping the document tree into LaTeX files. List of tuples
# (source start file, target name, title,
#  author, documentclass [howto, manual, or own class]).
latex_documents = [
  ('ez-manual', 'EZ-Draw-doc.tex', 'EZ-Draw Documentation',
   'Edouard Thiel', 'manual'),
]

# The name of an image file (relative to this directory) to place at the top of
# the title page.
#latex_logo = None

# For "manual" documents, if this is true, then toplevel headings are parts,
# not chapters.
#latex_use_parts = False

# If true, show page references after internal links.
#latex_show_pagerefs = False

# If true, show URL addresses after external links.
#latex_show_urls = False

# Documents to append as an appendix to all manuals.
#latex_appendices = []

# If false, no module index is generated.
#latex_domain_indices = True


# -- Options for manual page output ---------------------------------------

# One entry per manual page. List of tuples
# (source start file, name, description, authors, manual section).
man_pages = [
    ('ez-manual', 'ez-draw', 'EZ-Draw Documentation',
     ['Edouard Thiel'], 1)
]

# If true, show URL addresses after external links.
#man_show_urls = False


# -- Options for Texinfo output -------------------------------------------

# Grouping the document tree into Texinfo files. List of tuples
# (source start file, target name, title, author,
#  dir menu entry, description, category)
texinfo_documents = [
  ('ez-manual', 'EZ-Draw-doc', 'EZ-Draw Documentation',
   'Edouard Thiel', 'EZ-Draw', 'One line description of project.',
   'Miscellaneous'),
]

# Documents to append as an appendix to all manuals.
#texinfo_appendices = []

# If false, no module index is generated.
#texinfo_domain_indices = True

# How to display URL addresses: 'footnote', 'no', or 'inline'.
#texinfo_show_urls = 'footnote'

# If true, do not generate a @detailmenu in the "Top" node's menu.
#texinfo_no_detailmenu = False


# -- Common definitions ---------------------------------------------------

# EZ-Draw tarballs root page
ez_root_page = "http://pageperso.lif.univ-mrs.fr/~edouard.thiel"

# EZ-Draw current version
ez_version = "1.2"

# EZ-Draw source path, relative from doc/build/html/
ez_src_path = "../../../"

# A string of reStructuredText that will be included at the end of every source 
# file that is read.
rst_epilog = """

.. _ez-draw.c:  {path}ez-draw.c
.. _ez-draw.h:  {path}ez-draw.h
.. _ez-image.c: {path}ez-image.c
.. _ez-image.h: {path}ez-image.h

.. _Makefile:     {path}Makefile
.. _make.bat:     {path}make.bat

.. _demo-01.c: {path}demo-01.c
.. _demo-02.c: {path}demo-02.c
.. _demo-03.c: {path}demo-03.c
.. _demo-04.c: {path}demo-04.c
.. _demo-05.c: {path}demo-05.c
.. _demo-06.c: {path}demo-06.c
.. _demo-07.c: {path}demo-07.c
.. _demo-08.c: {path}demo-08.c
.. _demo-09.c: {path}demo-09.c
.. _demo-10.c: {path}demo-10.c
.. _demo-11.c: {path}demo-11.c
.. _demo-12.c: {path}demo-12.c
.. _demo-13.c: {path}demo-13.c
.. _demo-14.c: {path}demo-14.c
.. _demo-15.c: {path}demo-15.c
.. _demo-16.c: {path}demo-16.c
.. _demo-17.c: {path}demo-17.c

.. _jeu-nim.c: {path}jeu-nim.c

.. _tutorial: ez-tutorial.html
.. _installation: ez-install.html

.. |EZ-Draw-x.y| replace:: EZ-Draw-{ver}
.. |EZ-Draw-x.y.tgz| replace:: EZ-Draw-{ver}.tgz
.. _EZ-Draw-x.y.tgz: {root}/EZ-Draw-{ver}.tgz
.. |pre-EZ-Draw-x.y| replace:: ``EZ-Draw-{ver}``
.. |pre-EZ-Draw-x.y.tar| replace:: ``EZ-Draw-{ver}.tar``
.. |pre-EZ-Draw-x.y.tgz| replace:: ``EZ-Draw-{ver}.tgz``

.. |bullet| image:: ../share/pinkball.gif
            :alt: bullet

""".format(path=ez_src_path, root=ez_root_page, ver=ez_version)

# Examples
# .. |aa| replace:: foo
# .. |bb| replace:: bar
# .. |RST| replace:: |aa|\ |bb|
# .. _RST: http://docutils.sourceforge.net/rst.html
# The link for |RST| is |RST|_


