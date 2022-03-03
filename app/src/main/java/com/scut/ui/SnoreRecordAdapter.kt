package com.scut.ui

import android.icu.text.SimpleDateFormat
import android.icu.util.TimeZone
import android.view.LayoutInflater
import android.view.ViewGroup
import androidx.recyclerview.widget.RecyclerView
import com.scut.R
import com.scut.databinding.ItemSnoreBinding
import com.scut.model.SnoreRecord
import java.util.*

class SnoreRecordAdapter() : RecyclerView.Adapter<SnoreRecordAdapter.SnoreRecordViewHolder>() {
    var mList = emptyList<SnoreRecord>()

    class SnoreRecordViewHolder(binding: ItemSnoreBinding) : RecyclerView.ViewHolder(binding.root) {
        private val mBinding = binding
        private lateinit var mSnoreRecord: SnoreRecord

        fun bind(snoreRecord: SnoreRecord) {
            mSnoreRecord = snoreRecord
            val context = mBinding.root.context
            mBinding.snoreDate.text = context.getString(
                R.string.snore_date,
                SimpleDateFormat("HH:mm:ss.SSS", Locale.getDefault()).format(snoreRecord.timestamp)
            )
            mBinding.snoreDuration.text = context.getString(
                R.string.snore_duration,
                SimpleDateFormat("ss.SSS秒", Locale.UK).apply {
                    timeZone = TimeZone.getTimeZone("UTC")
                }.format(snoreRecord.length)
            )
        }
    }

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): SnoreRecordViewHolder {
        val binding = ItemSnoreBinding.inflate(LayoutInflater.from(parent.context), parent, false)
        return SnoreRecordViewHolder(binding)
    }

    override fun onBindViewHolder(holder: SnoreRecordViewHolder, position: Int) {
        holder.bind(mList[position])
    }

    override fun getItemCount(): Int {
        return mList.size
    }
}