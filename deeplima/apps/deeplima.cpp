// Copyright 2021 CEA LIST
// SPDX-FileCopyrightText: 2022 CEA LIST <gael.de-chalendar@cea.fr>
//
// SPDX-License-Identifier: MIT

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <functional>
#include <boost/program_options.hpp>

#include "version/version.h"
#include "helpers/path_resolver.h"

namespace po = boost::program_options;

void parse_file(std::istream& input,
                const std::map<std::string, std::string>& models_fn,
                const deeplima::PathResolver& path_resolver,
                size_t threads,
                size_t out_fmt=1);

int main(int argc, char* argv[])
{
  setlocale(LC_ALL, "en_US.UTF-8");
  std::cerr << "deeplima (git commit hash: " << deeplima::version::get_git_commit_hash() << ", "
       << "git branch: " << deeplima::version::get_git_branch()
       << ")" << std::endl;

  size_t threads = 1;
  std::string input_format, output_format, tok_model, tag_model, lem_model;
  std::vector<std::string> input_files;

  po::options_description desc("deeplima (analysis demo)");
  desc.add_options()
  ("help,h",                                                                             "Display this help message")
  ("input-format",    po::value<std::string>(&input_format)->default_value("plain"),     "Input format: plain|conllu")
  ("output-format",   po::value<std::string>(&output_format)->default_value("conllu"),   "Output format: conllu|vertical|horizontal")
  ("tok-model",       po::value<std::string>(&tok_model)->default_value(""),             "Tokenization model")
  ("tag-model",       po::value<std::string>(&tag_model)->default_value(""),             "Tagging model")
  ("lem-model",       po::value<std::string>(&lem_model)->default_value(""),             "Lemmatization model")
  ("input-file",      po::value<std::vector<std::string>>(&input_files),                 "Input file names")
  ("threads",         po::value<size_t>(&threads),                                       "Max threads to use")
  ;

  po::positional_options_description pos_desc;
  pos_desc.add("input-file", -1);

  po::variables_map vm;

  try
  {
    po::store(po::command_line_parser(argc, argv).options(desc).positional(pos_desc).run(), vm);
    po::notify(vm);
  }
  catch (const boost::program_options::unknown_option& e)
  {
    std::cerr << e.what() << std::endl;
    return -1;
  }

  if (vm.count("help")) {
      std::cout << desc << std::endl;
      return 0;
  }

  if (tok_model.empty() && tag_model.empty())
  {
    std::cerr << "No model is provided: --tok-model or --tag-model parameters are required." << std::endl << std::endl;
    std::cout << desc << std::endl;
    return -1;
  }

  std::map<std::string, std::string> models;

  if (tok_model.size() > 0)
  {
    models["tok"] = tok_model;
  }

  if (tag_model.size() > 0)
  {
    models["tag"] = tag_model;
  }

  if (lem_model.size() > 0)
  {
    models["lem"] = lem_model;
  }

  size_t out_fmt = 1;
  if (output_format.size() > 0)
  {
    if (output_format == "horizontal")
    {
      out_fmt = 2;
    }
  }

  deeplima::PathResolver path_resolver;

  if (vm.count("input-file") > 0)
  {
    for ( const auto& fn : input_files )
    {
      std::ifstream file(fn, std::ifstream::binary | std::ios::in);
      parse_file(file, models, path_resolver, threads, out_fmt);
    }
  }
  else
  {
    parse_file(std::cin, models, path_resolver, threads, out_fmt);
  }

  return 0;
}

#include "deeplima/segmentation.h"
#include "deeplima/ner.h"
#include "deeplima/token_sequence_analyzer.h"
#include "deeplima/dumper_conllu.h"
#include "deeplima/reader_conllu.h"

using namespace deeplima;

void parse_file(std::istream& input,
                const std::map<std::string, std::string>& models_fn,
                const PathResolver& path_resolver,
                size_t threads,
                size_t out_fmt)
{
  std::cerr << "threads = " << threads << std::endl;
  segmentation::ISegmentation* psegm = nullptr;

  if (models_fn.end() != models_fn.find("tok"))
  {
    psegm = new segmentation::Segmentation();
    static_cast<segmentation::Segmentation*>(psegm)->load(models_fn.find("tok")->second);
    static_cast<segmentation::Segmentation*>(psegm)->init(threads, 16*1024);
  }
  else
  {
    psegm = new segmentation::CoNLLUReader();
  }

  TokenSequenceAnalyzer<>* panalyzer = nullptr;
  dumper::AbstractDumper* pdumper = nullptr;
  dumper::AnalysisToConllU<typename TokenSequenceAnalyzer<>::TokenIterator> conllu_dumper;
  if (models_fn.end() != models_fn.find("tag"))
  {
    std::string lemm_model_fn;
    std::map<std::string, std::string>::const_iterator it = models_fn.find("lem");
    if (models_fn.end() != it)
    {
      lemm_model_fn = it->second;
    }

    panalyzer
        = new TokenSequenceAnalyzer<>(models_fn.find("tag")->second,
                                      lemm_model_fn, path_resolver, 1024, 8);

    for (size_t i = 0; i < panalyzer->get_classes().size(); ++i)
    {
      conllu_dumper.set_classes(i, panalyzer->get_class_names()[i], panalyzer->get_classes()[i]);
    }

    panalyzer->register_handler([&conllu_dumper](const StringIndex& stridx,
                                const token_buffer_t& tokens,
                                const std::vector<StringIndex::idx_t>& lemmata,
                                const typename TokenSequenceAnalyzer<>::OutputMatrix& classes,
                                size_t begin,
                                size_t end)
    {
      typename TokenSequenceAnalyzer<>::TokenIterator ti(stridx,
                                                         tokens,
                                                         lemmata,
                                                         classes,
                                                         begin,
                                                         end);
      conllu_dumper(ti);
    });

    psegm->register_handler([panalyzer]
                            (const std::vector<segmentation::token_pos>& tokens,
                             uint32_t len)
    {
      (*panalyzer)(tokens, len);
    });
  }
  else
  {
    switch (out_fmt) {
    case 1:
      pdumper = new dumper::TokensToConllU();
      break;
    case 2:
      pdumper = new dumper::Horizontal();
      break;
    default:
      throw std::runtime_error("Unknown output format");
      break;
    }
    psegm->register_handler([pdumper]
                            (const std::vector<segmentation::token_pos>& tokens,
                             uint32_t len)
    {
      (*pdumper)(tokens, len);
    });
  }

  std::chrono::steady_clock::time_point parsing_begin = std::chrono::steady_clock::now();

  psegm->parse_from_stream([&input]
                         (uint8_t* buffer,
                         uint32_t& read,
                         uint32_t max)
  {
    input.read((std::istream::char_type*)buffer, max);
    read = input.gcount();
    return (bool)input;
  });

  uint64_t token_counter = (nullptr != pdumper ? pdumper->get_token_counter() : 0);

  if (nullptr != panalyzer)
  {
    if (0 == token_counter)
    {
      token_counter = conllu_dumper.get_token_counter();
    }

    std::cerr << "Waiting for analyzer to stop" << std::endl;
    panalyzer->finalize();
    delete panalyzer;
    std::cerr << "Analyzer stopped" << std::endl;
  }
  std::chrono::steady_clock::time_point parsing_end = std::chrono::steady_clock::now();
  float parsing_duration = std::chrono::duration_cast<std::chrono::milliseconds>(parsing_end - parsing_begin).count();
  float speed = (float(token_counter) * 1000) / parsing_duration;
  std::cerr << "Parsing speed: " << speed << " tokens / sec" << std::endl;

  if (nullptr != psegm)
  {
    std::cerr << "Deleting psegm" << std::endl;
    delete psegm;
  }

  if (nullptr != pdumper)
  {
    delete pdumper;
  }

  if (!input.eof() && (input.fail() || input.bad()))
  {
    throw std::runtime_error("parse_file: error while reading the input file.");
  }
}

