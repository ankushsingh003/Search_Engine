#include "postings_list.hpp"
#include "vbyte.hpp"

namespace SearchEngine {

void PostingsList::add_occurrence(DocID doc_id, Position pos) {
    if (postings.empty() || postings.back().doc_id != doc_id) {
        postings.push_back({doc_id, 1, {pos}});
    } else {
        postings.back().term_freq++;
        postings.back().positions.push_back(pos);
    }
}

void PostingsList::serialize(std::vector<uint8_t>& out) const {
    VByteCodec::encode(static_cast<uint32_t>(postings.size()), out);
    DocID last_doc_id = 0;
    for (const auto& p : postings) {
        // Delta encode DocID
        VByteCodec::encode(p.doc_id - last_doc_id, out);
        last_doc_id = p.doc_id;

        // Encode Term Frequency
        VByteCodec::encode(p.term_freq, out);

        // Encode Positions with deltas
        Position last_pos = 0;
        for (Position pos : p.positions) {
            VByteCodec::encode(pos - last_pos, out);
            last_pos = pos;
        }
    }
}

PostingsList PostingsList::deserialize(const uint8_t*& ptr) {
    PostingsList pl;
    uint32_t num_postings = VByteCodec::decode(ptr);
    DocID last_doc_id = 0;
    for (uint32_t i = 0; i < num_postings; ++i) {
        DocID delta_doc = VByteCodec::decode(ptr);
        DocID doc_id = last_doc_id + delta_doc;
        last_doc_id = doc_id;

        uint32_t tf = VByteCodec::decode(ptr);
        Posting p{doc_id, tf, {}};
        
        Position last_pos = 0;
        for (uint32_t j = 0; j < tf; ++j) {
            Position delta_pos = VByteCodec::decode(ptr);
            Position pos = last_pos + delta_pos;
            p.positions.push_back(pos);
            last_pos = pos;
        }
        pl.postings.push_back(std::move(p));
    }
    return pl;
}

} // namespace SearchEngine
