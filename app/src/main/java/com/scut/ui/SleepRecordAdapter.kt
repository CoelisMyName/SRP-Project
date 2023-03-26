package com.scut.ui

import android.graphics.Color
import android.icu.text.SimpleDateFormat
import android.icu.util.TimeZone
import android.view.LayoutInflater
import android.view.ViewGroup
import androidx.recyclerview.widget.RecyclerView
import com.scut.R
import com.scut.databinding.ItemRecordBinding
import com.scut.model.SleepRecord
import java.util.*

/**
 * 本类用于显示历史记录列表中的一个条目
 */
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
            // 开始时间生成
            mBinding.sleepDate.text = SimpleDateFormat(
                "yyyy/MM/dd HH:mm:ss", Locale.getDefault()
            ).format(sleepRecord.timestamp)
            mBinding.sleepDuration.text = context.getString(
                R.string.log_duration,
                SimpleDateFormat("HH:mm:ss", Locale.UK).apply {
                    timeZone = TimeZone.getTimeZone("UTC")
                }.format(sleepRecord.duration)
            )
            // 鼾声次数显示
            mBinding.snoreTimes.text = when (sleepRecord.snoreTimes) {
                0L -> context.getString(R.string.snore_time_0)
                else -> context.getString(R.string.snore_times, sleepRecord.snoreTimes)
            }
            // 诊断结果显示
            mBinding.isPatient.text = when (sleepRecord.label) {
                0.0 -> context.getString(R.string.no)
                1.0 -> context.getString(R.string.yes)
                -1.0 -> context.getString(R.string.calculating)
                else -> context.getString(R.string.unexcept)
            }
            // 着色，无鼾声无标签为正常，有标签为危险，其他为警告
            var txtColor = R.color.state_warning_text
            var bgColor = R.color.state_warning_bg
            if (sleepRecord.label == 1.0) {
                txtColor = R.color.state_danger_text
                bgColor = R.color.state_danger_bg
            }
            else if ((sleepRecord.label == 0.0) and (sleepRecord.snoreTimes == 0L)) {
                txtColor = R.color.state_healthy_text
                bgColor = R.color.state_healthy_bg
            }
            val txtClr = context.getColor (txtColor)
            mBinding.sleepDate.setTextColor (txtClr)
            mBinding.sleepStartText.setTextColor (txtClr)
            mBinding.snoreTimes.setTextColor (txtClr)
            mBinding.sleepDuration.setTextColor (txtClr)
            mBinding.isPatient.setTextColor (txtClr)
            mBinding.recordCard.setCardBackgroundColor(context.getColor(bgColor))
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