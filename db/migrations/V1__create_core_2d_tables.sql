CREATE TABLE IF NOT EXISTS public.jobs (
    id                  BIGSERIAL       PRIMARY KEY,
    message_id          BIGINT          NOT NULL UNIQUE,
    study_uid           TEXT            NOT NULL,
    source_path         TEXT            NOT NULL,
    status              TEXT            NOT NULL DEFAULT 'queued',
    started_at_utc_ms   BIGINT,
    finished_at_utc_ms  BIGINT,
    processing_ms       BIGINT,
    gpu_device          TEXT,
    error_code          TEXT,
    error_detail        TEXT,
    created_at          TIMESTAMPTZ     NOT NULL DEFAULT NOW(),
    updated_at          TIMESTAMPTZ     NOT NULL DEFAULT NOW()
);

CREATE INDEX IF NOT EXISTS idx_core_2d_jobs_status  ON public.jobs (status);
CREATE INDEX IF NOT EXISTS idx_core_2d_jobs_study   ON public.jobs (study_uid);
CREATE INDEX IF NOT EXISTS idx_core_2d_jobs_created ON public.jobs (created_at DESC);

CREATE TABLE IF NOT EXISTS public.results (
    id              BIGSERIAL   PRIMARY KEY,
    job_id          BIGINT      NOT NULL REFERENCES public.jobs(id) ON DELETE CASCADE,
    metric_name     TEXT        NOT NULL,
    metric_value    DOUBLE PRECISION,
    payload         JSONB,
    created_at      TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE INDEX IF NOT EXISTS idx_core_2d_results_job ON public.results (job_id);