# SPIMI Algorithm (Single-Pass In-Memory Indexing)

## Overview

The SPIMI (Single-Pass In-Memory Indexing) algorithm is an efficient method for building inverted indices from large collections of documents. It allows for the processing of documents in a single pass while utilizing limited memory, making it suitable for handling big data applications. This implementation focuses on optimizing the indexing process by using a combination of in-memory data structures and file-based storage for large datasets.

## Key Features

- **Single-Pass Processing**: The algorithm processes each document only once, reducing the time complexity associated with indexing.
- **In-Memory Indexing**: Utilizes in-memory data structures to store intermediate results, which are then written to disk in manageable chunks.
- **Handling Large Datasets**: Capable of indexing large collections of documents that exceed memory capacity by breaking the data into smaller sub-indices.
- **Efficient Merging**: Combines multiple sub-indices into a final inverted index, ensuring that all document IDs are correctly aggregated.

## How It Works

1. **Document Processing**:
   - The algorithm reads documents from a specified directory.
   - Each document is tokenized, and terms are normalized using stemming to reduce variations.

2. **Index Creation**:
   - For each term, a list of document IDs is created, indicating where the term appears.
   - Intermediate indices are stored in separate files to manage memory usage effectively.

3. **Merging Indices**:
   - After processing all documents, the algorithm merges the individual sub-indices into a single inverted index.
   - Duplicates are eliminated, and the final index is written to a CSV file for easy access.
![SPIMI Algorithm Diagram](SPIMIPic1.png)

## Usage

1. Clone the repository:

   ```bash
   git clone git@github.com:lojain-azzam/SPIMI-.git
   cd SPIMI-
