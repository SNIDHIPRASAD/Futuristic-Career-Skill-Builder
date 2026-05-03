import streamlit as st
import pickle
import os
import sqlite3
import hashlib
import json
from datetime import datetime
import io, re
import numpy as np
from pathlib import Path
import textwrap

# =========================
# === PAGE CONFIG ===
# =========================
st.set_page_config(page_title="Futuristic Career & Skill Builder", layout="centered", page_icon="⚡")

# =========================
# === CSS STYLING ===
# =========================
st.markdown("""
<style>
body {
    background: radial-gradient(circle at 10% 20%, #0f2027, #203a43, #2c5364);
    color: #f5f5f5;
}

/* === ⚡ Futuristic Animated RGB Glow Frame === */
.rgb-border {
    position: relative;
    border-radius: 18px;
    margin: 28px 0;
    padding: 4px;
    background: linear-gradient(
        120deg,
        #ff0057,
        #fffd00,
        #00ff73,
        #00c3ff,
        #ff00f5,
        #ff0057
    );
    background-size: 300% 300%;
    animation: rgbFlow 5s linear infinite;
    box-shadow: 
        0 0 15px rgba(255, 0, 87, 0.4),
        0 0 30px rgba(0, 255, 115, 0.2),
        0 0 45px rgba(0, 195, 255, 0.15);
}

/* Inner card content */
.inner-box {
    position: relative;
    border-radius: 14px;
    background: rgba(10, 15, 30, 0.95);
    padding: 24px 28px;
    z-index: 1;
    box-shadow: inset 0 0 20px rgba(255,255,255,0.05);
    color: #f5f5f5;
    transition: transform 0.25s ease, box-shadow 0.25s ease;
}

/* Hover effect */
.rgb-border:hover {
    box-shadow: 
        0 0 25px rgba(255, 0, 87, 0.6),
        0 0 50px rgba(0, 255, 115, 0.4),
        0 0 75px rgba(0, 195, 255, 0.3);
}
.rgb-border:hover .inner-box {
    transform: translateY(-4px);
    box-shadow: inset 0 0 25px rgba(255,255,255,0.08);
}

/* Animation */
@keyframes rgbFlow {
    0% { background-position: 0% 50%; }
    50% { background-position: 100% 50%; }
    100% { background-position: 0% 50%; }
}

/* Buttons */
.stButton>button, div.stButton>button {
    background: linear-gradient(90deg, #ff0057, #00ff73, #00c3ff);
    background-size: 300% 300%;
    color: white;
    font-weight: bold;
    font-size: 16px;
    border: none;
    border-radius: 12px;
    padding: 10px 25px;
    transition: all 0.3s ease;
    box-shadow: 0 0 8px rgba(255,255,255,0.2);
}
.stButton>button:hover {
    animation: gradientGlow 2s linear infinite;
    box-shadow: 0 0 20px #ff00f5, 0 0 40px #00ff73, 0 0 60px #00c3ff;
}
@keyframes gradientGlow {
    0% { background-position: 0% 50%; }
    50% { background-position: 100% 50%; }
    100% { background-position: 0% 50%; }
}

/* Inputs */
.stTextInput>div>input {
    background-color: #1c1f3a;
    color: white;
    border-radius: 10px;
    border: 1px solid #333;
}

/* === Login Page Enhancements === */
.rgb-border-login {
    position: relative;
    border-radius: 18px;
    margin: 40px auto;
    max-width: 420px;
    padding: 4px;
    background: linear-gradient(
        120deg,
        #ff0057,
        #fffd00,
        #00ff73,
        #00c3ff,
        #ff00f5,
        #ff0057
    );
    background-size: 300% 300%;
    animation: rgbFlow 6s linear infinite;
    box-shadow: 
        0 0 15px rgba(255, 0, 87, 0.4),
        0 0 30px rgba(0, 255, 115, 0.2),
        0 0 45px rgba(0, 195, 255, 0.15);
}
.inner-box-login {
    background: rgba(10, 15, 30, 0.92);
    border-radius: 14px;
    padding: 30px;
    color: #f5f5f5;
    text-align: center;
    box-shadow: inset 0 0 15px rgba(255,255,255,0.05);
}

/* Custom radio button styling */
.stRadio > div {
    background: rgba(20, 25, 45, 0.8);
    border: 1px solid #333;
    border-radius: 10px;
    padding: 15px;
    margin: 10px 0;
    transition: all 0.3s ease;
}
.stRadio > div:hover {
    background: rgba(30, 35, 55, 0.9);
    border-color: #00ff73;
}
.stRadio > div[data-testid="stRadio"] > label {
    color: white !important;
    font-size: 16px;
    padding: 10px;
}
.stRadio > div[data-testid="stRadio"] > div {
    background: transparent !important;
}
</style>
""", unsafe_allow_html=True)

# =========================
# === SAVE/LOAD USERS LOCALLY ===
# =========================
USER_FILE = "user_data.pkl"

def save_data(data):
    with open(USER_FILE, "wb") as f:
        pickle.dump(data, f)

def load_data():
    if os.path.exists(USER_FILE):
        with open(USER_FILE, "rb") as f:
            return pickle.load(f)
    return {"users": {}, "user_chats": {}}

# Load user data
if "user_data" not in st.session_state:
    st.session_state.user_data = load_data()

st.session_state.users = st.session_state.user_data["users"]
st.session_state.user_chats = st.session_state.user_data["user_chats"]

# =========================
# === SESSION STATE DEFAULTS ===
# =========================
defaults = {
    "logged_in": False,
    "username": None,
    "stage": 0,
    "quiz_results": {},
    "recommended_path": None,
    "app_mode": "Career Path Finder",
    "messages": [],
    "last_mode": "Career Path Finder",
    "rag_history": [],
    "summary_text": "",
    "quiz_items": [],
    "quiz_choices": {},
    "quiz_graded": False,
    "quiz_feedback": {},
    # Career Advisor specific states
    "career_answers": {},
    "current_question_index": 0,
    "custom_options": {},  # Store custom options added by users
    "career_questions": [
        {
            'id': 'interest',
            'question': "What area of tech interests you most?",
            'type': 'choice_with_custom',
            'options': [
                "🤖 Artificial Intelligence & Machine Learning",
                "🌐 Web Development (Frontend/Backend)",
                "🔒 Cybersecurity & Information Security",
                "📊 Data Science & Analytics",
                "📱 Mobile App Development",
                "☁️ Cloud Computing & DevOps",
                "🎮 Game Development",
                "💻 Software Engineering",
                "🎨 UX/UI Design",
                "🤖 Robotics & IoT"
            ],
            'next': 'problem_type'
        },
        {
            'id': 'problem_type', 
            'question': "What kind of problems do you enjoy solving?",
            'type': 'choice_with_custom',
            'options': [
                "🧩 Logical puzzles and complex algorithms",
                "🎨 Visual design and creative challenges",
                "👥 People-oriented and communication problems",
                "🔍 Investigative and analytical mysteries",
                "⚡ Fast-paced troubleshooting and debugging",
                "📈 Data analysis and pattern recognition",
                "🛡️ Security and protection challenges",
                "🚀 Building products from scratch"
            ],
            'next': 'work_style'
        },
        {
            'id': 'work_style',
            'question': "Describe your ideal work environment:",
            'type': 'choice_with_custom',
            'options': [
                "💻 Solo deep work with minimal interruptions",
                "👨‍💻👩‍💻 Collaborative team with creative discussions",
                "⚡ Fast-paced startup with rapid changes",
                "🏢 Structured corporate with clear processes",
                "🏠 Remote work with flexible hours",
                "🔬 Research-focused with academic approach",
                "🎯 Project-based with clear milestones",
                "🔄 Agile environment with frequent iterations"
            ],
            'next': 'skills'
        },
        {
            'id': 'skills',
            'question': "What are your current strengths or skills you enjoy using?",
            'type': 'choice_with_custom',
            'options': [
                "💻 Programming and coding",
                "🎨 Design and creativity",
                "🗣️ Communication and teaching",
                "📊 Analysis and research",
                "🔧 Technical troubleshooting",
                "📝 Writing and documentation",
                "👥 Leadership and teamwork",
                "💡 Problem-solving and critical thinking",
                "📈 Project management",
                "🎯 Attention to detail"
            ],
            'next': 'goal'
        },
        {
            'id': 'goal', 
            'question': "What's your primary career goal?",
            'type': 'choice_with_custom',
            'options': [
                "🚀 Build innovative products and applications",
                "📊 Analyze data to drive business decisions",
                "🛡️ Protect systems and ensure security",
                "👥 Lead and manage technical teams",
                "💼 Start my own tech company",
                "🎯 Become a technical expert in a specific field",
                "🌍 Make a positive social impact through tech",
                "💰 Earn a high salary in a stable company",
                "🎓 Continuously learn and research new technologies"
            ],
            'next': None
        }
    ]
}
for key, val in defaults.items():
    if key not in st.session_state:
        st.session_state[key] = val

# =========================
# === DATABASE HELPERS ===
# =========================
DB_PATH = "app.db"

def get_db():
    conn = sqlite3.connect(DB_PATH, check_same_thread=False)
    conn.execute("PRAGMA foreign_keys = ON;")
    return conn

def init_db():
    conn = get_db()
    cur = conn.cursor()
    cur.execute("""
    CREATE TABLE IF NOT EXISTS users (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        email TEXT UNIQUE NOT NULL,
        display_name TEXT,
        password_hash TEXT NOT NULL,
        salt TEXT NOT NULL,
        created_at TEXT NOT NULL
    );
    """)
    cur.execute("""
    CREATE TABLE IF NOT EXISTS user_state (
        user_id INTEGER PRIMARY KEY,
        stage INTEGER,
        recommended_path TEXT,
        quiz_results_json TEXT,
        updated_at TEXT NOT NULL,
        FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
    );
    """)
    cur.execute("""
    CREATE TABLE IF NOT EXISTS chat_messages (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        user_id INTEGER NOT NULL,
        path_label TEXT,
        ts TEXT NOT NULL,
        role TEXT NOT NULL,
        content TEXT NOT NULL,
        FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
    );
    """)
    cur.execute("""
    CREATE TABLE IF NOT EXISTS rag_history (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        user_id INTEGER NOT NULL,
        ts TEXT NOT NULL,
        question TEXT NOT NULL,
        answer TEXT NOT NULL,
        FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
    );
    """)
    cur.execute("""
    CREATE TABLE IF NOT EXISTS quiz_wrong_answers (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        user_id INTEGER NOT NULL,
        ts TEXT NOT NULL,
        question TEXT NOT NULL,
        options_json TEXT NOT NULL,
        correct_index INTEGER NOT NULL,
        chosen_index INTEGER NOT NULL,
        FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
    );
    """)
    conn.commit()
    conn.close()

def pbkdf2_hash(password: str, salt_bytes: bytes) -> str:
    return hashlib.pbkdf2_hmac('sha256', password.encode('utf-8'), salt_bytes, 200_000).hex()

def create_user(email: str, display_name: str, password: str):
    conn = get_db()
    cur = conn.cursor()
    salt_bytes = os.urandom(16)
    salt_hex = salt_bytes.hex()
    pwd_hash = pbkdf2_hash(password, salt_bytes)
    try:
        cur.execute(
            "INSERT INTO users (email, display_name, password_hash, salt, created_at) VALUES (?, ?, ?, ?, ?)",
            (email.lower().strip(), display_name.strip(), pwd_hash, salt_hex, datetime.utcnow().isoformat())
        )
        conn.commit()
        user_id = cur.lastrowid
        cur.execute(
            "INSERT INTO user_state (user_id, stage, recommended_path, quiz_results_json, updated_at) VALUES (?, ?, ?, ?, ?)",
            (user_id, 0, None, json.dumps({}), datetime.utcnow().isoformat())
        )
        conn.commit()
        return True, "Account created successfully. You can log in now."
    except sqlite3.IntegrityError:
        return False, "Email already registered."
    finally:
        conn.close()

def verify_user(email: str, password: str):
    conn = get_db()
    cur = conn.cursor()
    cur.execute("SELECT id, display_name, password_hash, salt FROM users WHERE email = ?", (email.lower().strip(),))
    row = cur.fetchone()
    conn.close()
    if not row:
        return None
    user_id, display_name, stored_hash, salt_hex = row
    calc_hash = pbkdf2_hash(password, bytes.fromhex(salt_hex))
    if calc_hash == stored_hash:
        return {"id": user_id, "email": email.lower().strip(), "display_name": display_name}
    return None

def load_user_state(user_id: int):
    conn = get_db()
    cur = conn.cursor()
    cur.execute("SELECT stage, recommended_path, quiz_results_json FROM user_state WHERE user_id = ?", (user_id,))
    row = cur.fetchone()
    conn.close()
    if not row:
        return 0, None, {}
    stage, path, quiz_json = row
    try:
        quiz = json.loads(quiz_json) if quiz_json else {}
    except Exception:
        quiz = {}
    return stage or 0, path, quiz

def save_user_state(user_id: int, stage: int, recommended_path: str, quiz_results: dict):
    conn = get_db()
    cur = conn.cursor()
    cur.execute("""
        INSERT INTO user_state (user_id, stage, recommended_path, quiz_results_json, updated_at)
        VALUES (?, ?, ?, ?, ?)
        ON CONFLICT(user_id) DO UPDATE SET
            stage=excluded.stage,
            recommended_path=excluded.recommended_path,
            quiz_results_json=excluded.quiz_results_json,
            updated_at=excluded.updated_at
    """, (user_id, stage, recommended_path, json.dumps(quiz_results or {}), datetime.utcnow().isoformat()))
    conn.commit()
    conn.close()

def save_message(user_id: int, role: str, content: str, path_label: str = None):
    conn = get_db()
    cur = conn.cursor()
    cur.execute("""
        INSERT INTO chat_messages (user_id, path_label, ts, role, content)
        VALUES (?, ?, ?, ?, ?)
    """, (user_id, path_label, datetime.utcnow().isoformat(), role, content))
    conn.commit()
    conn.close()

def load_messages(user_id: int, path_label: str = None):
    conn = get_db()
    cur = conn.cursor()
    if path_label:
        cur.execute("""
            SELECT role, content FROM chat_messages
            WHERE user_id = ? AND (path_label = ? OR path_label IS NULL)
            ORDER BY id ASC
        """, (user_id, path_label))
    else:
        cur.execute("""
            SELECT role, content FROM chat_messages
            WHERE user_id = ?
            ORDER BY id ASC
        """, (user_id,))
    rows = cur.fetchall()
    conn.close()
    return [{"role": r, "content": c} for (r, c) in rows]

def save_rag_turn(user_id: int, question: str, answer: str):
    conn = get_db()
    cur = conn.cursor()
    cur.execute("""
        INSERT INTO rag_history (user_id, ts, question, answer)
        VALUES (?, ?, ?, ?)
    """, (user_id, datetime.utcnow().isoformat(), question, answer))
    conn.commit()
    conn.close()

def load_rag_history(user_id: int):
    conn = get_db()
    cur = conn.cursor()
    cur.execute("""
        SELECT role, content FROM chat_messages
        WHERE user_id=? AND path_label='RAG' ORDER BY id ASC
    """, (user_id,))
    rows = cur.fetchall()
    conn.close()
    hist = []
    q = None
    for role, content in rows:
        if role == "user":
            q = content
        elif role == "assistant" and q:
            hist.append({"q": q, "a": content})
            q = None
    return hist

def merge_rag_history(session_hist, db_hist):
    """Merge session and DB rag history without duplicates (by Q text)."""
    seen = set()
    merged = []
    for entry in db_hist + session_hist:
        q = entry.get("q", "").strip().lower()
        if q and q not in seen:
            seen.add(q)
            merged.append(entry)
    return merged

def save_wrong_answers(user_id: int, items: list[dict], choices: dict[int, int]):
    """Persist only the incorrect questions for this user."""
    if not items: 
        return
    conn = get_db()
    cur = conn.cursor()
    now = datetime.utcnow().isoformat()
    for i, it in enumerate(items):
        qi = int(choices.get(i, -1))
        if qi < 0:  # unanswered counts as wrong
            qi = -1
        correct = int(it.get("answer_index", -1))
        if qi != correct:
            cur.execute("""
                INSERT INTO quiz_wrong_answers
                (user_id, ts, question, options_json, correct_index, chosen_index)
                VALUES (?, ?, ?, ?, ?, ?)
            """, (
                user_id, now,
                it.get("question", ""),
                json.dumps(it.get("options", []), ensure_ascii=False),
                correct, qi
            ))
    conn.commit()
    conn.close()

def load_recent_wrong_answers(user_id: int, limit: int = 25):
    conn = get_db()
    cur = conn.cursor()
    cur.execute("""
        SELECT ts, question, options_json, correct_index, chosen_index
        FROM quiz_wrong_answers
        WHERE user_id = ?
        ORDER BY id DESC
        LIMIT ?
    """, (user_id, limit))
    rows = cur.fetchall()
    conn.close()
    out = []
    for ts, q, opts_json, ci, ui in rows:
        out.append({
            "ts": ts, "question": q,
            "options": json.loads(opts_json), "answer_index": ci, "chosen_index": ui
        })
    return out

def _render_wrong_answers_panel(user_id: int, limit: int = 25):
    wrongs = load_recent_wrong_answers(user_id, limit=limit)
    if not wrongs:
        st.caption("No wrong answers saved yet.")
        return
    for row in wrongs:
        st.write(row["question"])
        opts = row["options"]
        ci = row["answer_index"]; ui = row["chosen_index"]
        st.write(f"Your answer: {(opts[ui] if 0 <= ui < 4 else '—')}")
        st.write(f"Correct answer: {opts[ci]}")
        st.markdown("---")

# =========================
# === RAG HELPERS ===
# =========================
RAG_BASE = Path("rag_store")

def _ensure_packages():
    missing = []
    try:
        import faiss
    except Exception:
        missing.append("faiss-cpu")
    try:
        from sentence_transformers import SentenceTransformer
    except Exception:
        missing.append("sentence-transformers")
    try:
        import PyPDF2
    except Exception:
        missing.append("PyPDF2")
    try:
        import docx
    except Exception:
        missing.append("python-docx")
    if missing:
        st.warning("Missing packages: " + ", ".join(missing) + ". Install via: pip install " + " ".join(missing))
    return not missing

def _read_pdf(file_bytes: bytes) -> str:
    import PyPDF2
    reader = PyPDF2.PdfReader(io.BytesIO(file_bytes))
    text = []
    for p in reader.pages:
        t = p.extract_text() or ""
        text.append(t)
    return "\n".join(text)

def _read_docx(file_bytes: bytes) -> str:
    import docx
    bio = io.BytesIO(file_bytes)
    d = docx.Document(bio)
    return "\n".join(p.text for p in d.paragraphs)

def _read_txt(file_bytes: bytes) -> str:
    try:
        return file_bytes.decode("utf-8", errors="ignore")
    except Exception:
        return file_bytes.decode("latin-1", errors="ignore")

def extract_text_from_file(uploaded) -> str:
    name = uploaded.name.lower()
    data = uploaded.read()
    if name.endswith(".pdf"):
        return _read_pdf(data)
    if name.endswith(".docx"):
        return _read_docx(data)
    return _read_txt(data)

def chunk_text(text: str, chunk_size=800, overlap=150):
    words = text.split()
    if not words:
        return []
    chunks = []
    i = 0
    while i < len(words):
        chunk = " ".join(words[i:i+chunk_size])
        chunks.append(chunk)
        i += (chunk_size - overlap)
    return chunks

def _embedder():
    from sentence_transformers import SentenceTransformer
    model = SentenceTransformer("all-MiniLM-L6-v2")
    return model

def compute_embeddings(chunks: list[str]) -> np.ndarray:
    model = _embedder()
    vecs = model.encode(chunks, convert_to_numpy=True, show_progress_bar=False, normalize_embeddings=True)
    return vecs.astype(np.float32)

def _faiss_import():
    import faiss
    return faiss

def _user_store(user_id: int) -> Path:
    p = RAG_BASE / str(user_id)
    p.mkdir(parents=True, exist_ok=True)
    return p

def _save_store(user_id: int, vectors: np.ndarray, chunks: list[dict], id_map: list[int]):
    faiss = _faiss_import()
    store = _user_store(user_id)
    index_path = store / "index.faiss"
    meta_path  = store / "chunks.json"
    map_path   = store / "idmap.npy"

    index = faiss.IndexFlatIP(vectors.shape[1])
    index.add(vectors)
    faiss.write_index(index, str(index_path))
    with open(meta_path, "w", encoding="utf-8") as f:
        json.dump(chunks, f, ensure_ascii=False, indent=2)
    np.save(map_path, np.array(id_map, dtype=np.int32))

def _load_store(user_id: int):
    faiss = _faiss_import()
    store = _user_store(user_id)
    index_path = store / "index.faiss"
    meta_path  = store / "chunks.json"
    map_path   = store / "idmap.npy"
    if not (index_path.exists() and meta_path.exists() and map_path.exists()):
        return None, None, None
    index = faiss.read_index(str(index_path))
    with open(meta_path, "r", encoding="utf-8") as f:
        chunks = json.load(f)
    id_map = np.load(map_path)
    return index, chunks, id_map

def _append_to_store(user_id: int, new_vecs: np.ndarray, new_chunks: list[dict]):
    index, chunks, id_map = _load_store(user_id)
    if index is None:
        _save_store(user_id, new_vecs, new_chunks, list(range(len(new_chunks))))
        return
    faiss = _faiss_import()
    dim = index.d
    if new_vecs.shape[1] != dim:
        st.error("Embedding dimension mismatch. Start a fresh knowledge base.")
        return
    all_chunks = chunks + new_chunks
    texts = [c["text"] for c in all_chunks]
    vecs = compute_embeddings(texts)
    _save_store(user_id, vecs, all_chunks, list(range(len(all_chunks))))

def build_kb_from_uploads(user_id: int, uploads: list):
    if not _ensure_packages():
        return 0
    docs = []
    sources = []
    for up in uploads:
        txt = extract_text_from_file(up)
        if not txt.strip():
            continue
        pieces = chunk_text(txt)
        docs.extend(pieces)
        sources.extend([{"file": up.name, "text": c} for c in pieces])
    if not docs:
        return 0
    vecs = compute_embeddings(docs)
    _append_to_store(user_id, vecs, sources)
    return len(docs)

def retrieve(user_id: int, query: str, k=5):
    if not _ensure_packages():
        return []
    index, chunks, _ = _load_store(user_id)
    if index is None:
        return []
    qvec = compute_embeddings([query])
    D, I = index.search(qvec, min(k, index.ntotal))
    I = I[0].tolist(); D = D[0].tolist()
    results = []
    for rank, (idx, score) in enumerate(zip(I, D), start=1):
        if 0 <= idx < len(chunks):
            r = chunks[idx]
            results.append({
                "rank": rank,
                "file": r.get("file",""),
                "text": r.get("text",""),
                "score": float(score)
            })
    return results

def retrieve_best(user_id: int, query: str):
    top1 = retrieve(user_id, query, k=1)
    if not top1:
        return []
    best = top1[0]
    if best["score"] < 0.25:
        top3 = retrieve(user_id, query, k=3)
        return top3[:1] if top3 else []
    return [best]

def _format_history(max_turns=3):
    hist = st.session_state.get("rag_history", [])[-max_turns:]
    if not hist:
        return ""
    lines = []
    for i, t in enumerate(hist, 1):
        lines.append(f"Turn {i} - Q: {t['q']}\nA: {t['a']}")
    return "\n".join(lines)

# =========================
# === GEMINI LLM HELPERS ===
# =========================
MODEL_NAME = "gemini-2.5-flash"

def _get_api_key():
    """Read API key from KEY.txt (preferred) or environment."""
    for path in ("KEY.txt", "./KEY.txt", "../KEY.txt"):
        if os.path.exists(path):
            try:
                with open(path, "r", encoding="utf-8") as f:
                    key = f.read().strip()
                    if key:
                        return key
            except Exception:
                pass
    return os.environ.get("GEMINI_API_KEY") or os.environ.get("GOOGLE_API_KEY")

@st.cache_resource(show_spinner=False)
def _get_gemini_model():
    try:
        import google.generativeai as genai
    except ImportError:
        st.error("❌ Google Generative AI package not installed. Run: pip install google-generativeai")
        return None
        
    api_key = _get_api_key()
    if not api_key:
        st.warning("⚠ Gemini API key missing. Put it in KEY.txt or set GEMINI_API_KEY.")
        return None
        
    try:
        genai.configure(api_key=api_key)
        return genai.GenerativeModel(
            model_name=MODEL_NAME,
            system_instruction=(
                "You are a helpful teaching assistant. "
                "Answer ONLY from the provided context. "
                'If the answer is not in the context, reply with "I don\'t know." '
                "Keep responses clear, factual, and concise."
            ),
        )
    except Exception as e:
        st.error(f"❌ Error loading Gemini model: {e}")
        return None

def _build_context_from_hits(hits: list[dict], max_chars=12000):
    parts, used = [], 0
    for h in hits[:5]:
        block = f"[Source: {h.get('file','(unknown)')} | Rank {h['rank']}]\n{h['text']}\n"
        if used + len(block) > max_chars:
            block = block[: max(0, max_chars - used)]
        parts.append(block)
        used += len(block)
        if used >= max_chars:
            break
    return "\n\n".join(parts).strip()

def _llm_answer_from_hits(question: str, hits: list[dict]) -> str:
    model = _get_gemini_model()
    if not model:
        return "AI is not configured (missing API key or package)."
    context = _build_context_from_hits(hits, max_chars=12000)
    if not context:
        return "I couldn't find relevant content in your notes."
    history_txt = _format_history(max_turns=3)
    history_block = f"\nPrevious turns (for continuity, do not cite beyond provided context):\n{history_txt}\n" if history_txt else ""
    prompt = (
        "You are answering questions for a student/teacher based ONLY on the provided context.\n"
        "If the answer is not fully supported, say you don't know.\n"
        f"{history_block}"
        "Context:\n"
        f"{context}\n\n"
        "Task: Using ONLY the context above, answer the user's question clearly. "
        "If the context is insufficient, say you don't know.\n\n"
        f"Question: {question}"
        )

    try:
        resp = model.generate_content(prompt)
        answer = (getattr(resp, "text", "") or "").strip() or "I couldn't produce an answer."
        return answer
    except Exception as e:
        return f"Model error: {e}"

def _llm_summarize_corpus(chunks: list[dict]) -> str:
    model = _get_gemini_model()
    if not model:
        return "AI is not configured (missing API key or package)."
    rough = "\n".join(c["text"] for c in chunks)
    context = textwrap.shorten(rough, width=12000, placeholder=" …")
    prompt = (
        "Refine the following rough extractive summary into a faithful, well-structured overview. "
        "Avoid hallucination and keep it grounded. Add 3–6 bullet key takeaways at the end.\n\n"
        f"{context}"
    )
    try:
        resp = model.generate_content(prompt)
        return (getattr(resp, "text", "") or "").strip()
    except Exception as e:
        return f"Model error: {e}"

def ensure_local_user(username: str) -> int:
    """Ensure a matching row exists in 'users' and return its id."""
    conn = get_db()
    cur = conn.cursor()
    cur.execute("SELECT id FROM users WHERE email=?", (username,))
    row = cur.fetchone()
    if row:
        uid = row[0]
    else:
        salt_bytes = os.urandom(16)
        salt_hex = salt_bytes.hex()
        pwd_hash = pbkdf2_hash(username, salt_bytes)
        cur.execute(
            "INSERT INTO users (email, display_name, password_hash, salt, created_at) VALUES (?, ?, ?, ?, ?)",
            (username, username, pwd_hash, salt_hex, datetime.utcnow().isoformat()),
        )
        conn.commit()
        uid = cur.lastrowid
        cur.execute(
            "INSERT INTO user_state (user_id, stage, recommended_path, quiz_results_json, updated_at) VALUES (?, ?, ?, ?, ?)",
            (uid, 0, None, json.dumps({}), datetime.utcnow().isoformat()),
        )
        conn.commit()
    conn.close()
    return uid

def _llm_quiz_from_corpus(chunks: list[dict], n_q=5):
    model = _get_gemini_model()
    if not model:
        return [{"question": "AI not configured (missing key or package).", "options": ["-", "-", "-", "-"], "answer_index": 0}]
    
    corpus = "\n".join(c["text"] for c in chunks)
    context = textwrap.shorten(corpus, width=12000, placeholder=" …")

    prompt = (
        f"From the context below, create exactly {n_q} multiple-choice questions (MCQs). "
        "Each question must have 4 options and one correct answer index. "
        "Return the result strictly in JSON format, for example:\n"
        "{\"items\": [{\"question\": \"Q1 text\", \"options\": [\"A\", \"B\", \"C\", \"D\"], \"answer_index\": 2}]}\n"
        "Use only information found in the context.\n\n"
        f"Context:\n{context}"
    )

    try:
        resp = model.generate_content(prompt)
        txt = (getattr(resp, "text", "") or "").strip()

        import re, json
        m = re.search(r'\{[\s\S]*\}', txt)
        data = json.loads(m.group(0)) if m else json.loads(txt)

        if "items" in data and isinstance(data["items"], list):
            return data["items"]
        elif isinstance(data, list):
            return data
        elif isinstance(data, dict) and all(k in data for k in ("question", "options", "answer_index")):
            return [data]
        else:
            return []
    except Exception as e:
        return [{"question": f"Quiz generation error: {e}", "options": ["-", "-", "-", "-"], "answer_index": 0}]

# =========================
# === CAREER ADVISOR FUNCTIONS ===
# =========================
def save_answer(question_id, answer):
    """Save answer and move to appropriate next question"""
    st.session_state.career_answers[question_id] = answer
    
    # Find current question to determine next
    for i, q in enumerate(st.session_state.career_questions):
        if q['id'] == question_id:
            next_question_id = q.get('next')
            if next_question_id:
                # Find and go to next question
                for j, next_q in enumerate(st.session_state.career_questions):
                    if next_q['id'] == next_question_id:
                        st.session_state.current_question_index = j
                        break
            else:
                # No more questions - show recommendations
                st.session_state.current_question_index = len(st.session_state.career_questions)
            break

def add_custom_option(question_id, custom_option):
    """Add a custom option to the question's options list"""
    if question_id not in st.session_state.custom_options:
        st.session_state.custom_options[question_id] = []
    
    if custom_option and custom_option not in st.session_state.custom_options[question_id]:
        st.session_state.custom_options[question_id].append(custom_option)

def get_all_options(question_data):
    """Get all options including predefined and custom ones"""
    question_id = question_data['id']
    base_options = question_data.get('options', [])
    custom_options = st.session_state.custom_options.get(question_id, [])
    return base_options + custom_options

def show_choice_with_custom_question(question_data):
    """Display a question with choice options and custom input"""
    question_id = question_data['id']
    current_answer = st.session_state.career_answers.get(question_id, '')
    all_options = get_all_options(question_data)
    
    # Display the question
    st.markdown(f"### {question_data['question']}")
    
    # Use Streamlit's native radio button with all options
    selected_option = st.radio(
        "Choose one option:",
        options=all_options,
        index=all_options.index(current_answer) if current_answer in all_options else 0,
        key=f"radio_{question_id}"
    )
    
    # Custom option input
    st.markdown("---")
    st.markdown("**Don't see your preferred option? Add your own:**")
    custom_input_col1, custom_input_col2 = st.columns([3, 1])
    
    with custom_input_col1:
        custom_option = st.text_input(
            "Enter custom option:",
            key=f"custom_input_{question_id}",
            placeholder="Type your own answer here..."
        )
    
    with custom_input_col2:
        if st.button("➕ Add", key=f"add_custom_{question_id}"):
            if custom_option.strip():
                add_custom_option(question_id, custom_option.strip())
                st.success(f"Added: {custom_option.strip()}")
                st.rerun()
            else:
                st.warning("Please enter a custom option")
    
    # Navigation buttons
    col1, col2 = st.columns([1, 1])
    
    with col1:
        if st.session_state.current_question_index > 0:
            if st.button("⬅️ Previous", key=f"prev_btn_{question_id}"):
                st.session_state.current_question_index -= 1
                st.rerun()
    
    with col2:
        if selected_option:
            if st.button("Next ➡️", key=f"next_btn_{question_id}"):
                save_answer(question_id, selected_option)
                st.rerun()
        else:
            st.button("Next ➡️", key=f"next_btn_disabled_{question_id}", disabled=True)
            st.info("👆 Please select an option or add your own to continue")

def generate_career_recommendations():
    """Generate career recommendations based on user answers"""
    answers = st.session_state.career_answers
    
    # Extract answers
    interest = answers.get('interest', '').lower()
    problem_type = answers.get('problem_type', '').lower()
    work_style = answers.get('work_style', '').lower()
    skills = answers.get('skills', '').lower()
    goal = answers.get('goal', '').lower()
    
    recommendations = []
    
    # AI/ML focus
    if any(word in interest for word in ['ai', 'artificial', 'machine learning', 'neural', 'deep learning']):
        if any(word in problem_type for word in ['data', 'analy', 'pattern', 'research']):
            recommendations.append({
                "title": "🤖 Machine Learning Engineer",
                "description": "Build and deploy intelligent systems that learn from data to solve complex problems",
                "skills": ["Python", "TensorFlow/PyTorch", "Data Analysis", "Statistics", "ML Algorithms"],
                "path": "Python fundamentals → Data analysis → ML frameworks → Model deployment → Cloud ML services",
                "match_score": "95% match"
            })
        else:
            recommendations.append({
                "title": "🧠 AI Research Scientist", 
                "description": "Push the boundaries of artificial intelligence through cutting-edge research and innovation",
                "skills": ["Advanced Mathematics", "Research Methods", "Python", "Academic Writing", "Algorithm Design"],
                "path": "Strong math foundation → Research experience → Advanced degrees → Industry research labs",
                "match_score": "90% match"
            })
    
    # Web Development focus
    if any(word in interest for word in ['web', 'website', 'frontend', 'backend', 'full stack']):
        if any(word in problem_type for word in ['visual', 'design', 'creative', 'user']):
            recommendations.append({
                "title": "🎨 Frontend Developer",
                "description": "Create beautiful, responsive user interfaces that provide exceptional user experiences",
                "skills": ["HTML/CSS", "JavaScript", "React/Vue/Angular", "UI/UX Design", "Responsive Design"],
                "path": "HTML/CSS basics → JavaScript mastery → Frontend frameworks → Build portfolio → Advanced animations",
                "match_score": "92% match"
            })
        if any(word in problem_type for word in ['logic', 'data', 'system', 'server']):
            recommendations.append({
                "title": "⚙️ Backend Developer",
                "description": "Build robust server-side systems, APIs, and databases that power applications",
                "skills": ["Node.js/Python/Java", "Databases", "APIs", "System Design", "Cloud Services"],
                "path": "Programming fundamentals → Database design → API development → System architecture → Cloud deployment",
                "match_score": "88% match"
            })
    
    # Cybersecurity focus  
    if any(word in interest for word in ['cyber', 'security', 'hack', 'protect', 'information security']):
        recommendations.append({
            "title": "🛡️ Cybersecurity Analyst",
            "description": "Protect systems and networks from digital attacks and ensure data security",
            "skills": ["Network Security", "Ethical Hacking", "Risk Assessment", "Incident Response", "Security Tools"],
            "path": "Networking basics → Security fundamentals → Certifications (Security+, CEH) → Specialized training",
            "match_score": "85% match"
        })
    
    # Data focus
    if any(word in interest for word in ['data', 'analy', 'business intelligence', 'analytics']):
        recommendations.append({
            "title": "📊 Data Scientist",
            "description": "Extract insights from complex data sets to drive business decisions and innovation",
            "skills": ["Python/R", "SQL", "Data Visualization", "Statistics", "Machine Learning"],
            "path": "SQL mastery → Data analysis → Statistical modeling → ML applications → Big Data tools",
            "match_score": "90% match"
        })
    
    # Default recommendations based on problem-solving style
    if not recommendations:
        if any(word in problem_type for word in ['logic', 'puzzle', 'algorithm', 'technical']):
            recommendations.append({
                "title": "💻 Software Engineer",
                "description": "Solve complex problems through elegant code and scalable system design",
                "skills": ["Algorithms", "Data Structures", "System Design", "Multiple Languages", "Testing"],
                "path": "Programming fundamentals → Data structures → System design → Specialization → Architecture",
                "match_score": "82% match"
            })
        elif any(word in problem_type for word in ['visual', 'design', 'creative', 'user experience']):
            recommendations.append({
                "title": "✨ UX/UI Designer",
                "description": "Create intuitive and beautiful user experiences that delight users",
                "skills": ["User Research", "Wireframing", "Prototyping", "Design Tools", "User Testing"],
                "path": "Design principles → Tools mastery → Portfolio building → User research → Specialization",
                "match_score": "85% match"
            })
        elif any(word in problem_type for word in ['people', 'help', 'communication', 'team']):
            recommendations.append({
                "title": "👥 Product Manager", 
                "description": "Bridge business needs with technical solutions and lead product development",
                "skills": ["Communication", "Project Management", "Market Analysis", "Technical Understanding", "Leadership"],
                "path": "Domain knowledge → Agile methodologies → Technical basics → Leadership → Strategy",
                "match_score": "80% match"
            })
        else:
            recommendations.append({
                "title": "🚀 Tech Generalist",
                "description": "Explore multiple areas to find your true passion and build diverse skills",
                "skills": ["Adaptability", "Learning Agility", "Broad Technical Knowledge", "Problem Solving"],
                "path": "Try different areas → Build diverse projects → Identify interests → Specialize gradually",
                "match_score": "75% match"
            })
    
    return recommendations

def show_career_advisor():
    """Main career advisor interface"""
    st.title("🎯 Interactive Career Advisor")
    st.markdown("Discover your ideal tech career path through personalized questions")
    
    current_index = st.session_state.current_question_index
    total_questions = len(st.session_state.career_questions)
    
    # Show progress and navigation
    col1, col2, col3 = st.columns([2, 1, 1])
    with col1:
        if current_index < total_questions:
            progress = current_index / total_questions
            st.progress(progress)
            st.caption(f"Question {current_index + 1} of {total_questions}")
    
    # Question navigation
    with col2:
        if st.button("🔄 Restart Quiz", key="restart_quiz_btn"):
            st.session_state.career_answers = {}
            st.session_state.current_question_index = 0
            st.session_state.custom_options = {}
            st.rerun()
    
    with col3:
        if st.session_state.career_answers and current_index < total_questions:
            if st.button("📋 Review Answers", key="review_answers_btn"):
                st.session_state.show_review = True
                st.rerun()
    
    # Show current question or recommendations
    if current_index < total_questions:
        show_current_question()
    else:
        show_recommendations()

def show_current_question():
    """Display the current question based on its type"""
    current_index = st.session_state.current_question_index
    question_data = st.session_state.career_questions[current_index]
    
    st.markdown("---")
    
    if question_data['type'] == 'choice_with_custom':
        show_choice_with_custom_question(question_data)

def show_recommendations():
    """Display career recommendations based on user answers"""
    st.markdown("---")
    st.markdown("## 🎉 Your Personalized Career Recommendations")
    
    # Show summary of user answers
    with st.expander("📝 Review Your Answers", expanded=False):
        for i, question in enumerate(st.session_state.career_questions):
            answer = st.session_state.career_answers.get(question['id'], 'Not answered')
            st.write(f"**{question['question']}**")
            st.write(f"*{answer}*")
            if question['id'] in st.session_state.custom_options:
                st.caption(f"Custom options added: {', '.join(st.session_state.custom_options[question['id']])}")
            st.markdown("---")
    
    # Generate and display recommendations
    recommendations = generate_career_recommendations()
    
    st.markdown("### 🚀 Recommended Career Paths")
    
    for i, rec in enumerate(recommendations):
        with st.container():
            # Header with match score
            col_header1, col_header2 = st.columns([3, 1])
            with col_header1:
                st.markdown(f"#### {rec['title']}")
            with col_header2:
                st.markdown(f"**{rec.get('match_score', '80% match')}**")
            
            st.write(rec['description'])
            
            col1, col2 = st.columns([1, 1])
            
            with col1:
                st.markdown("**🔧 Key Skills:**")
                for skill in rec['skills']:
                    st.write(f"• {skill}")
            
            with col2:
                st.markdown("**🎯 Learning Path:**")
                steps = rec['path'].split(' → ')
                for j, step in enumerate(steps, 1):
                    st.write(f"{j}. {step}")
            
            st.markdown("---")
    
    # Action buttons
    col1, col2, col3 = st.columns(3)
    
    with col1:
        if st.button("🔄 Start Over", key="start_over_btn"):
            st.session_state.career_answers = {}
            st.session_state.current_question_index = 0
            st.session_state.custom_options = {}
            st.rerun()
    
    with col2:
        if st.button("📋 Review Answers", key="final_review_btn"):
            st.session_state.current_question_index = 0
            st.rerun()
    
    with col3:
        if st.button("💬 Chat About Paths", key="chat_paths_btn"):
            st.session_state.app_mode = "Interactive Chatbot"
            st.rerun()

# =========================
# === LOGIN / SIGNUP SYSTEM ===
# =========================
def show_login():
    try:
        st.image("ARCH_logo_placeholder.jpg")
    except FileNotFoundError:
        st.markdown("<h3 style='text-align:center;color:#ccc;'>[Image Placeholder]</h3>", unsafe_allow_html=True)

    st.markdown("""
    <h1 style='text-align:center; color:#00ffcc; text-shadow:0 0 15px #00ffcc;'>⚡ Futuristic Career Portal</h1>
    <p style='text-align:center; color:#aaa;'>Welcome to the <b>AI-powered Career & Skill Builder</b>.<br>Login or create your account to begin your personalized journey.</p>
    """, unsafe_allow_html=True)

    st.markdown("""
    <div class="rgb-border-login"><div class="inner-box-login">
        <h3 style='color:#fffd88;'>🔐 Sign In / Create Account</h3>
    """, unsafe_allow_html=True)

    with st.form("login_form", clear_on_submit=False):
        username = st.text_input("👤 Username", placeholder="Enter your username")
        password = st.text_input("🔑 Password", type="password", placeholder="Enter your password")

        col1, col2 = st.columns(2)
        login_btn = col1.form_submit_button("Login", key="login_form_btn")
        signup_btn = col2.form_submit_button("Sign Up", key="signup_form_btn")

    st.markdown("</div></div>", unsafe_allow_html=True)

    if login_btn:
        if username in st.session_state.users and st.session_state.users[username] == password:
            st.session_state.logged_in = True
            st.session_state.username = username
            st.session_state.messages = st.session_state.user_chats.get(username, [])
            st.session_state.db_user_id = ensure_local_user(username)
            st.success(f"✅ Welcome back, {username}!")
            st.rerun()
        else:
            st.error("❌ Invalid credentials. Try again.")

    if signup_btn:
        if username in st.session_state.users:
            st.warning("⚠ User already exists.")
        elif not username or not password:
            st.warning("⚠ Please fill in both fields.")
        else:
            st.session_state.users[username] = password
            st.session_state.user_chats[username] = []
            st.session_state.logged_in = True
            st.session_state.username = username
            st.session_state.db_user_id = ensure_local_user(username)
            save_data(st.session_state.user_data)
            st.success(f"🎉 Account created successfully! Welcome, {username}!")
            st.rerun()

# =========================
# === LOGOUT / SIGNOUT ===
# =========================
def logout():
    user = st.session_state.username
    if user:
        st.session_state.user_chats[user] = st.session_state.messages
        save_data({"users": st.session_state.users, "user_chats": st.session_state.user_chats})
    st.session_state.logged_in = False
    st.session_state.username = None
    st.session_state.messages = []
    st.rerun()

def signout():
    user = st.session_state.username
    if user in st.session_state.users:
        del st.session_state.users[user]
    if user in st.session_state.user_chats:
        del st.session_state.user_chats[user]
    save_data({"users": st.session_state.users, "user_chats": st.session_state.user_chats})
    st.session_state.logged_in = False
    st.session_state.username = None
    st.session_state.messages = []
    st.rerun()

# =========================
# === INTERACTIVE CHATBOT ===
# =========================
def show_interactive_chatbot():
    st.title("🤖 Interactive Skill Builder")
    for msg in st.session_state.messages:
        with st.chat_message(msg["role"]):
            st.markdown(msg["content"])
    if prompt := st.chat_input("Ask something..."):
        st.session_state.messages.append({"role": "user", "content": prompt})
        with st.spinner("Thinking..."):
            response = "Chat feature coming soon! Complete the Career Path Finder first."
        st.session_state.messages.append({"role": "assistant", "content": response})
        st.rerun()
    if st.button("← Back to Career Advisor", key="back_to_career_btn"):
        st.session_state.app_mode = "Career Path Finder"
        st.rerun()

# =========================
# === DATABASE & DOCS ===
# =========================
def show_database_and_docs():
    st.title("📚 Document RAG: Summarize • Q&A • Quiz (FAISS + Gemini)")
    st.markdown("---")
    st.info("Upload PDFs/DOCX/TXT → local FAISS index. AI uses ONLY retrieved passages (K=5) to answer/summarize. Exactly one API call per action.")

    if not st.session_state.logged_in:
        st.warning("🔐 Log in to build and use your personal knowledge base.")
        return

    user_id = st.session_state.get("db_user_id") or ensure_local_user(st.session_state.username)
    K = 5

    with st.expander("Step 1 · Upload / Update Notes", expanded=True):
        uploads = st.file_uploader("Upload notes (PDF, DOCX, TXT)", type=["pdf","docx","txt"], accept_multiple_files=True)
        if st.button("📥 Build / Update Knowledge Base", use_container_width=True, key="build_kb_btn"):
            if not uploads:
                st.warning("Please upload at least one file.")
            else:
                n = build_kb_from_uploads(user_id, uploads)
                if n > 0:
                    st.success(f"Added/updated {n} chunks into your KB.")
                else:
                    st.info("No content parsed. Check your files.")

    index, chunks, _ = _load_store(user_id)

    # ---------- AI Summary ----------
    st.subheader("🧾 Summarize Your Notes")
    make_summary = st.button("🧠 AI Summary (notes-only)", use_container_width=True, disabled=index is None, key="summary_btn")
    if make_summary:
        if index is None or not chunks:
            st.warning("No knowledge base found. Upload and build first.")
        else:
            with st.spinner("Summarizing..."):
                st.session_state.summary_text = _llm_summarize_corpus(chunks)
    # Always show the last summary if we have one
    if st.session_state.summary_text:
        st.success("Summary")
        st.write(st.session_state.summary_text)
    st.markdown("---")

    # ---------- AI Q&A Chat (notes-grounded) ----------
    st.subheader("💬 Ask a Question (grounded to your notes)")
    hist = st.session_state.get("rag_history", [])
    try:
        db_hist = load_rag_history(user_id)
        hist = merge_rag_history(hist, db_hist)
        st.session_state.rag_history = hist
    except Exception:
        pass
    if st.session_state.get("rag_history"):
        with st.expander("🗂 Previous Questions & Answers", expanded=False):
            for turn in st.session_state["rag_history"][-10:]:
                st.markdown(f"Q: {turn['q']}")
                st.markdown(f"A: {turn['a']}")
                st.markdown("---")
    
    q = st.text_input("Question", placeholder="e.g., Explain PID controller tuning rules", key="question_input")
    if st.button("🔎 AI Answer (most-relevant passage)", use_container_width=True, disabled=index is None, key="answer_btn"):
        if not q.strip():
            st.warning("Type a question.")
        else:
            with st.spinner("Retrieving & answering..."):
                hits = retrieve(user_id, q, k=8) or []
                if not hits:
                    st.info("No matches found. Try adding more notes.")
                else:
                    answer = _llm_answer_from_hits(q, hits)
                    st.success("Answer")
                    st.write(answer)
                    st.session_state.rag_history.append({"q": q, "a": answer})
                    with st.expander("Source (retrieved passage)"):
                        h = hits[0]
                        st.markdown(f"{h['rank']}.** {h.get('file','(unknown)')}")
                        st.caption(h["text"][:550] + ("..." if len(h["text"])>550 else ""))
    st.markdown("---")

    # ---------- AI Quiz ----------
    st.subheader("🧩 Auto-Generate Quiz")
    n_q = st.number_input("Number of questions", min_value=3, max_value=20, value=5, step=1, key="quiz_count")
    col_q1, col_q2, col_q3 = st.columns([1,1,0.6])
    with col_q1:
        make_quiz = st.button("📝 AI Quiz (notes-only)", use_container_width=True, disabled=index is None, key="make_quiz_btn")
    with col_q2:
        submit_quiz = st.button("✅ Submit Answers", use_container_width=True, disabled=not st.session_state.quiz_items, key="submit_quiz_btn")
    with col_q3:
        # Wrong answers panel
        if st.session_state.logged_in:
            try:
                with st.popover("Wrong answers", use_container_width=True):
                    _render_wrong_answers_panel(user_id, limit=25)
            except Exception:
                if "show_wrong_pop" not in st.session_state:
                    st.session_state.show_wrong_pop = False
                if st.button("Wrong answers", use_container_width=True, key="wrong_answers_btn"):
                    st.session_state.show_wrong_pop = not st.session_state.show_wrong_pop
                if st.session_state.show_wrong_pop:
                    with st.expander("Recent wrong answers", expanded=True):
                        _render_wrong_answers_panel(user_id, limit=25)

    if make_quiz:
        if index is None or not chunks:
            st.warning("No knowledge base found. Upload and build first.")
        else:
            with st.spinner("Generating quiz..."):
                items = _llm_quiz_from_corpus(chunks, n_q=int(n_q)) or []
            if not items:
                st.info("Couldn't generate MCQs. Add more content.")
            else:
                # reset state for a fresh run
                st.session_state.quiz_items = items
                st.session_state.quiz_choices = {}
                st.session_state.quiz_graded = False
                st.session_state.quiz_feedback = {}
                st.success(f"Quiz ready with {len(items)} questions. Scroll down to answer.")

    # Render quiz if present
    items = st.session_state.quiz_items
    if items:
        st.markdown("---")
        correct_count = 0
        for i, it in enumerate(items):
            qtext = it.get("question", f"Question {i+1}")
            options = it.get("options", ["-", "-", "-", "-"])
            st.markdown(f"Q{i+1}. {qtext}")
            # keep user's selection in session_state
            key = f"quiz_choice_{i}"
            default_index = st.session_state.quiz_choices.get(i, None)
            choice = st.radio(
                "Choose one:",
                options=[f"{j+1}. {opt}" for j, opt in enumerate(options)],
                index=(default_index if default_index is not None else 0),
                key=key,
                horizontal=False,
                label_visibility="collapsed"
            )
            # store back chosen index
            chosen_idx = int(choice.split(".")[0]) - 1
            st.session_state.quiz_choices[i] = chosen_idx
            # if graded, show feedback inline
            if st.session_state.quiz_graded:
                correct_idx = int(it.get("answer_index", 0))
                is_correct = (chosen_idx == correct_idx)
                if is_correct:
                    correct_count += 1
                    st.success(f"✅ Correct: {options[correct_idx]}")
                else:
                    st.error(f"❌ Incorrect. Correct answer: {options[correct_idx]}")
            st.markdown("---")
        if st.session_state.quiz_graded:
            st.info(f"Score: {correct_count}/{len(items)}")
    
    # Handle submit & persist wrong answers
    if submit_quiz and items:
        st.session_state.quiz_graded = True
        # persist user mistakes (and unanswered) for logged-in users
        if st.session_state.logged_in:
            try:
                save_wrong_answers(
                    user_id,
                    items,
                    st.session_state.quiz_choices
                    )
            except Exception as e:
                st.warning(f"Could not save wrong answers: {e}")
        st.rerun()

# =========================
# === SIDEBAR ===
# =========================
if st.session_state.logged_in:
    with st.sidebar:
        try:
            st.image("ARCH_logo_placeholder.jpg")
        except FileNotFoundError:
            st.write("ARCH Logo Placeholder")
        st.markdown(f"### 👋 {st.session_state.username}")
        st.button("🔓 Logout (Keep Data)", on_click=logout, key="sidebar_logout_btn")
        st.button("❌ Delete Account", on_click=signout, key="sidebar_delete_btn")
        st.markdown("---")
        selected = st.radio("🛠 Select Phase/Tool:", ["Career Path Finder", "Interactive Chatbot", "Database & Docs"], index=["Career Path Finder", "Interactive Chatbot", "Database & Docs"].index(st.session_state.app_mode), key="app_mode_radio")
        st.session_state.app_mode = selected
else:
    show_login()
    st.stop()

# =========================
# === MAIN ROUTER ===
# =========================
if st.session_state.app_mode == "Career Path Finder":
    show_career_advisor()

elif st.session_state.app_mode == "Interactive Chatbot":
    show_interactive_chatbot()

elif st.session_state.app_mode == "Database & Docs":
    show_database_and_docs()

# Initialize database
init_db()
