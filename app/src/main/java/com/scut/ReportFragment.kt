package com.scut

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import com.scut.databinding.FragmentReportBinding
import com.scut.databinding.ItemReportBinding


class ReportFragment : Fragment() {

    private lateinit var mBinding: FragmentReportBinding
    private lateinit var mAdapter: MyRecycleViewAdapter

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        mBinding = FragmentReportBinding.inflate(inflater, container, false)

        mBinding.recycler.setHasFixedSize(true)
        mAdapter = MyRecycleViewAdapter(30)
        val layoutManager = LinearLayoutManager(requireContext())
        mBinding.recycler.layoutManager = layoutManager
        mBinding.recycler.adapter = mAdapter

        return mBinding.root
    }
}

//自定义的ViewHolder
class MyHolder(binding: ItemReportBinding) : RecyclerView.ViewHolder(binding.root) {
    private val mBinding = binding

    fun bind(string: String) {
        mBinding.date.text = "1"
        mBinding.duration.text = "2"
        mBinding.times.text = "3"
        mBinding.weak.text = "4"
    }
}

class MyRecycleViewAdapter(num: Int) : RecyclerView.Adapter<MyHolder>() {
    //创建ViewHolder并返回，后续item布局里控件都是从ViewHolder中取出

    private val mNum = num

    override fun onCreateViewHolder(
        parent: ViewGroup,
        viewType: Int
    ): MyHolder {
        val binding = ItemReportBinding.inflate(LayoutInflater.from(parent.context), parent, false)
        return MyHolder(binding)
    }

    override fun onBindViewHolder(holder: MyHolder, position: Int) {
        holder.bind("")
    }

    //获取数据源总的条数
    override fun getItemCount(): Int {
        return mNum
    }
}

