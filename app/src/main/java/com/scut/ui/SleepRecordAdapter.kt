package com.scut.ui

import android.view.LayoutInflater
import android.view.ViewGroup
import androidx.recyclerview.widget.RecyclerView
import com.scut.databinding.ItemRecordBinding
import com.scut.model.SleepRecord

class SleepRecordAdapter() : RecyclerView.Adapter<SleepRecordAdapter.SleepRecordViewHolder>() {

    var mList = emptyList<SleepRecord>()

    class SleepRecordViewHolder(binding: ItemRecordBinding) :
        RecyclerView.ViewHolder(binding.root) {
        private val mBinding = binding
        private lateinit var mSleepRecord: SleepRecord

        fun bind(sleepRecord: SleepRecord) {
            mSleepRecord = sleepRecord
            //TODO show sleep record
        }
    }

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): SleepRecordViewHolder {
        val binding = ItemRecordBinding.inflate(LayoutInflater.from(parent.context), parent, false)
        return SleepRecordViewHolder(binding)
    }

    override fun onBindViewHolder(holder: SleepRecordViewHolder, position: Int) {
        holder.bind(mList[position])
    }

    override fun getItemCount(): Int {
        return mList.size
    }
}