package com.scut.ui

import android.icu.text.SimpleDateFormat
import android.icu.util.TimeZone
import android.view.LayoutInflater
import android.view.ViewGroup
import androidx.recyclerview.widget.RecyclerView
import com.scut.R
import com.scut.databinding.ItemRecordBinding
import com.scut.model.SleepRecord
import java.util.*

class SleepRecordAdapter(cb: SleepRecordCallback) :
    RecyclerView.Adapter<SleepRecordAdapter.SleepRecordViewHolder>() {
    private val mCallback = cb

    var mList = emptyList<SleepRecord>()

    class SleepRecordViewHolder(binding: ItemRecordBinding, cb: SleepRecordCallback) :
        RecyclerView.ViewHolder(binding.root) {
        private val mBinding = binding
        private val mCallback = cb
        private lateinit var mSleepRecord: SleepRecord

        init {
            mBinding.root.setOnClickListener { mCallback.showDetail(mSleepRecord) }
        }

        fun bind(sleepRecord: SleepRecord) {
            mSleepRecord = sleepRecord
            val context = mBinding.root.context
            mBinding.sleepDate.text = context.getString(
                R.string.sleep_test_date,
                SimpleDateFormat(
                    "yyyy/MM/dd HH:mm:ss",
                    Locale.getDefault()
                ).format(sleepRecord.timestamp)
            )
            mBinding.sleepDuration.text = context.getString(
                R.string.sleep_duration,
                SimpleDateFormat("HH:mm:ss", Locale.UK).apply {
                    timeZone = TimeZone.getTimeZone("UTC")
                }.format(sleepRecord.duration)
            )
            mBinding.snoreTimes.text =
                context.getString(R.string.snore_times, sleepRecord.snoreTimes)
            val isPatient = when (sleepRecord.label) {
                0.0 -> context.getString(R.string.no)
                1.0 -> context.getString(R.string.yes)
                -1.0 -> context.getString(R.string.calculating)
                else -> {
                    ""
                }
            }
            mBinding.isPatient.text = context.getString(R.string.is_patient_or_not, isPatient)
        }
    }

    interface SleepRecordCallback {
        fun showDetail(sl: SleepRecord)
    }

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): SleepRecordViewHolder {
        val binding = ItemRecordBinding.inflate(LayoutInflater.from(parent.context), parent, false)
        return SleepRecordViewHolder(binding, mCallback)
    }

    override fun onBindViewHolder(holder: SleepRecordViewHolder, position: Int) {
        holder.bind(mList[position])
    }

    override fun getItemCount(): Int {
        return mList.size
    }
}